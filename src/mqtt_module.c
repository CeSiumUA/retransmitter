#include "mqtt_module.h"

const char * const mqtt_client_name = "retransmitter_1";
const char * const mqtt_topic = "retransmitter/temperature";

static struct mqtt_client client;
static uint8_t sendbuf[2048] = {0};
static uint8_t recvbuf[2048] = {0};
static int sockfd = -1;
static bool refresh = true;
static pthread_t refresher_thread;

static int open_socket_nonblock(const char * broker, const char * port);
static void publish_callback(void** unused, struct mqtt_response_publish *published);
static void* client_refresher(void* client);

int mqtt_module_init(const char * broker, const char * port){
    enum MQTTErrors err;
    sockfd = open_socket_nonblock(broker, port);
    if(sockfd == -1){
        syslog(LOG_ERR, "Error: Could not open socket to broker\n");
        return -1;
    }

    err = mqtt_init(&client, sockfd, sendbuf, sizeof(sendbuf), recvbuf, sizeof(recvbuf), publish_callback);
    if(err != MQTT_OK){
        syslog(LOG_ERR, "Error: Could not initialize MQTT client\n");
        goto err_exit;
    }

    uint8_t connect_flags = MQTT_CONNECT_CLEAN_SESSION;
    err = mqtt_connect(&client, mqtt_client_name, NULL, NULL, 0, NULL, NULL, connect_flags, 400);
    if(err != MQTT_OK){
        syslog(LOG_ERR, "Error: Could not connect to broker\n");
        goto err_exit;
    }

    syslog(LOG_INFO, "Connected to broker\n");

    if(pthread_create(&refresher_thread, NULL, client_refresher, &client) != 0){
        syslog(LOG_ERR, "Error: Could not create refresher thread\n");
        goto err_exit;
    }

    err = mqtt_subscribe(&client, mqtt_topic, 0);
    if(err != MQTT_OK){
        syslog(LOG_ERR, "Error: Could not subscribe to topic\n");
        goto err_exit;
    }

    return 0;

err_exit:
    close(sockfd);
    return -1;
}

void mqtt_module_deinit(void){
    refresh = false;
    pthread_join(refresher_thread, NULL);
    mqtt_disconnect(&client);
    close(sockfd);
}

int mqtt_module_publish(const char * topic, const char * message){
    enum MQTTErrors err;
    err = mqtt_publish(&client, topic, message, strlen(message) + 1, MQTT_PUBLISH_QOS_0);
    if(err != MQTT_OK){
        syslog(LOG_ERR, "Error: Could not publish message\n");
        return -1;
    }

    return 0;
}

static void publish_callback(void** unused, struct mqtt_response_publish *published){
    (void)unused;

    char* topic_name = (char*) malloc(published->topic_name_size + 1);
    memcpy(topic_name, published->topic_name, published->topic_name_size);
    topic_name[published->topic_name_size] = '\0';
    syslog(LOG_INFO, "Received message (topic: '%s'): %s\n", topic_name, (const char *)published->application_message);

    free(topic_name);
}

static int open_socket_nonblock(const char * broker, const char * port){
    struct addrinfo hints = {0};
    struct addrinfo *p, *servinfo;

    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    int sockfd = -1;
    int res;

    syslog(LOG_DEBUG, "Connecting to broker %s:%s\n", broker, port);

    res = getaddrinfo(broker, port, &hints, &servinfo);
    if(res != 0) {
        syslog(LOG_ERR, "Error: Could not get address info (%s)\n", gai_strerror(res));
        return -1;
    }

    for(p = servinfo; p != NULL; p = p->ai_next) {
        sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
        if(sockfd == -1) {
            continue;
        }

        res = connect(sockfd, p->ai_addr, p->ai_addrlen);
        if(res == -1) {
            close(sockfd);
            sockfd = -1;
            continue;
        }

        break;
    }

    freeaddrinfo(servinfo);

    if(sockfd != -1){
        fcntl(sockfd, F_SETFL, fcntl(sockfd, F_GETFL) | O_NONBLOCK);
    }

    return sockfd;
}

static void* client_refresher(void* client) {
    while(refresh) {
        mqtt_sync((struct mqtt_client*) client);
        usleep(100000U);
    }
    return NULL;
}