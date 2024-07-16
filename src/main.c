#include "main.h"

static bool running = true;
static void load_configuration(const char *configuration_file, struct retransmitter_configuration_t *nrf24_configuration);
static void signal_handler(int sig);
static void mqtt_message_received_callback(const char *topic, const char *message);
static void read_write_loop(void);
char *nrf24_message_to_write = NULL;
pthread_mutex_t nrf24_message_mutex;

int main(int argc, char **argv) {
    struct retransmitter_configuration_t retransmitter_configuration = {
        .channel = NRF24_CONFIGURATION_CHANNEL_DEFAULT,
        .rx_payload_size = NRF24_CONFIGURATION_RX_PAYLOAD_SIZE_DEFAULT,
        .data_rate = NRF24_CONFIGURATION_DATA_RATE_DEFAULT,
        .data_pipe = NRF24_CONFIGURATION_DATA_PIPE_DEFAULT,
        .mqtt_broker = MQTT_CONFIGURATION_BROKER_DEFAULT,
        .mqtt_port = MQTT_CONFIGURATION_PORT_DEFAULT
    };
    char *configuration_file = NULL;
    struct sigaction sa = {
        .sa_handler = signal_handler,
        .sa_flags = 0
    };
    int res = 0;

    if(argc > 1) {
        configuration_file = argv[1];
    } else {
        configuration_file = CONFIGURATION_FILE_PATH;
    }

    openlog(argv[0], LOG_PID, LOG_DAEMON);

    sigemptyset(&sa.sa_mask);

    sigaction(SIGINT, &sa, NULL);
    sigaction(SIGTERM, &sa, NULL);

    syslog(LOG_DEBUG, "Loading configuration\n");
    load_configuration(configuration_file, &retransmitter_configuration);

    syslog(LOG_DEBUG, "Configuration loaded\n");

    syslog(LOG_DEBUG, "Daemonizing process\n");
    res = daemon(0, 0);
    if(res != 0) {
        syslog(LOG_ERR, "Error: Could not daemonize the process (%s)\n", strerror(errno));
        goto mqtt_exit;
    }

    res = pthread_mutex_init(&nrf24_message_mutex, NULL);
    if(res != 0) {
        syslog(LOG_ERR, "Error: Could not initialize mutex\n");
        goto exit;
    }

    syslog(LOG_DEBUG, "Initializing MQTT module\n");
    res = mqtt_module_init(retransmitter_configuration.mqtt_broker, retransmitter_configuration.mqtt_port, mqtt_message_received_callback);
    if(res != 0) {
        syslog(LOG_ERR, "Error: Could not initialize MQTT module\n");
        goto mutex_exit;
    }

    res = nrf24_open(retransmitter_configuration.channel,
                retransmitter_configuration.rx_payload_size,
                retransmitter_configuration.data_rate,
                retransmitter_configuration.data_pipe);

    if(res != 0) {
        syslog(LOG_ERR, "Error: Could not initialize NRF24 module\n");
        goto mqtt_exit;
    }

    read_write_loop();

    nrf24_close();
mqtt_exit:
    mqtt_module_deinit();
mutex_exit:
    pthread_mutex_destroy(&nrf24_message_mutex);
exit:
    closelog();
    return res;
}

static void read_write_loop(void) {
    int res = 0;
    char buffer[32] = {0};
    while(running) {
        res = nrf24_read((uint8_t *)buffer, sizeof(buffer));
        if(res > 0) {
            syslog(LOG_INFO, "Message received: %s\n", buffer);
            res = mqtt_module_publish(MQTT_MODULE_TEMPERATURE_TOPIC, buffer);
            if(res != 0) {
                syslog(LOG_ERR, "Error: Could not publish message\n");
            }
        }

        usleep(10);

        if(nrf24_message_to_write != NULL){
            syslog(LOG_INFO, "Writing message: %s\n", nrf24_message_to_write);
            pthread_mutex_lock(&nrf24_message_mutex);
            res = nrf24_write((uint8_t *)nrf24_message_to_write, strlen(nrf24_message_to_write) + 1);
            if(res < 0) {
                syslog(LOG_ERR, "Error: Could not write message\n");
            }
            free(nrf24_message_to_write);
            nrf24_message_to_write = NULL;
            pthread_mutex_unlock(&nrf24_message_mutex);
        }
    }
}

static void load_configuration(const char *configuration_file, struct retransmitter_configuration_t *retransmitter_configuration) {
    FILE *file = fopen(configuration_file, "r");
    if(file != NULL) {
        char line[256] = {0};
        while(fgets(line, sizeof(line), file)) {
            line[strcspn(line, "\n")] = 0;
            char *key = strtok(line, "=");
            char *value = strtok(NULL, "=");
            if(strcmp(key, "channel") == 0) {
                retransmitter_configuration->channel = atoi(value);
            } else if(strcmp(key, "rx_payload_size") == 0) {
                retransmitter_configuration->rx_payload_size = atoi(value);
            } else if(strcmp(key, "data_rate") == 0) {
                retransmitter_configuration->data_rate = atoi(value);
            } else if(strcmp(key, "data_pipe") == 0) {
                retransmitter_configuration->data_pipe = atoi(value);
            } else if(strcmp(key, "mqtt_broker") == 0) {
                strncpy(retransmitter_configuration->mqtt_broker, value, sizeof(retransmitter_configuration->mqtt_broker));
            } else if(strcmp(key, "mqtt_port") == 0) {
                strncpy(retransmitter_configuration->mqtt_port, value, sizeof(retransmitter_configuration->mqtt_port));
            }
            memset(line, 0, sizeof(line));
        }

        fclose(file);
    }
    else{
        syslog(LOG_INFO, "Error: Could not open configuration file %s (file does not exists or read not permitted)\n", configuration_file);
    }

    const char *env_channel = getenv(NRF24_CONFIGURATION_CHANNEL_ENV_NAME);
    if(env_channel != NULL) {
        syslog(LOG_DEBUG, "env channel value: %s\n", env_channel);
        retransmitter_configuration->channel = atoi(env_channel);
    }

    const char *env_rx_payload_size = getenv(NRF24_CONFIGURATION_RX_PAYLOAD_SIZE_ENV_NAME);
    if(env_rx_payload_size != NULL) {
        syslog(LOG_DEBUG, "env rx_payload_size value: %s\n", env_rx_payload_size);
        retransmitter_configuration->rx_payload_size = atoi(env_rx_payload_size);
    }

    const char *env_data_pipe = getenv(NRF24_CONFIGURATION_DATA_PIPE_ENV_NAME);
    if(env_data_pipe != NULL) {
        syslog(LOG_DEBUG, "env data_pipe value: %s\n", env_data_pipe);
        retransmitter_configuration->data_pipe = atoi(env_data_pipe);
    }

    const char *env_data_rate = getenv(NRF24_CONFIGURATION_DATA_RATE_ENV_NAME);
    if(env_data_rate != NULL) {
        syslog(LOG_DEBUG, "env data_rate value: %s\n", env_data_rate);
        retransmitter_configuration->data_rate = atoi(env_data_rate);
    }

    const char *env_mqtt_broker = getenv(MQTT_CONFIGURATION_BROKER_ENV_NAME);
    if(env_mqtt_broker != NULL) {
        syslog(LOG_DEBUG, "env mqtt_broker value: %s\n", env_mqtt_broker);
        strncpy(retransmitter_configuration->mqtt_broker, env_mqtt_broker, sizeof(retransmitter_configuration->mqtt_broker));
    }

    const char *env_mqtt_port = getenv(MQTT_CONFIGURATION_PORT_ENV_NAME);
    if(env_mqtt_port != NULL) {
        syslog(LOG_DEBUG, "env mqtt_port value: %s\n", env_mqtt_port);
        strncpy(retransmitter_configuration->mqtt_port, env_mqtt_port, sizeof(retransmitter_configuration->mqtt_port));
    }
}

static void signal_handler(int sig) {
    switch(sig) {
        case SIGINT:
        case SIGTERM:
            syslog(LOG_INFO, "Signal received, exiting\n");
            syslog(LOG_DEBUG, "Deinitializing MQTT module\n");
            mqtt_module_deinit();
            syslog(LOG_DEBUG, "Closing log\n");
            closelog();
            exit(EXIT_SUCCESS);
            break;
        default:
            break;
    }
}

static void mqtt_message_received_callback(const char *topic, const char *message) {
    syslog(LOG_INFO, "Message received on topic %s: %s\n", topic, message);
    if(strcmp(topic, MQTT_MODULE_GET_TEMPERATURE_TOPIC) == 0) {
        pthread_mutex_lock(&nrf24_message_mutex);
        nrf24_message_to_write = strdup(message);
        if(nrf24_message_to_write == NULL) {
            syslog(LOG_ERR, "Error: Could not allocate memory for message\n");
        }
        pthread_mutex_unlock(&nrf24_message_mutex);
    }
}