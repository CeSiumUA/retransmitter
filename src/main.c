#include "main.h"

static void load_configuration(const char *configuration_file, struct retransmitter_configuration_t *nrf24_configuration);
static void signal_handler(int sig);

int main(int argc, char **argv) {
    struct retransmitter_configuration_t retransmitter_configuration = {
        .channel = NRF24_CONFIGURATION_CHANNEL_DEFAULT,
        .rx_payload_size = NRF24_CONFIGURATION_RX_PAYLOAD_SIZE_DEFAULT,
        .data_rate = NRF24_CONFIGURATION_DATA_RATE_DEFAULT,
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

    res = daemon(0, 0);
    if(res != 0) {
        syslog(LOG_ERR, "Error: Could not daemonize the process (%s)\n", strerror(errno));
        goto exit;
    }

exit:
    closelog();
    return res;
}

static void load_configuration(const char *configuration_file, struct retransmitter_configuration_t *retransmitter_configuration) {
    FILE *file = fopen(configuration_file, "r");
    if(file != NULL) {
        char line[256];
        while(fgets(line, sizeof(line), file)) {
            char *key = strtok(line, "=");
            char *value = strtok(NULL, "=");
            if(strcmp(key, "channel") == 0) {
                retransmitter_configuration->channel = atoi(value);
            } else if(strcmp(key, "rx_payload_size") == 0) {
                retransmitter_configuration->rx_payload_size = atoi(value);
            } else if(strcmp(key, "data_rate") == 0) {
                retransmitter_configuration->data_rate = atoi(value);
            } else if(strcmp(key, "mqtt_broker") == 0) {
                strncpy(retransmitter_configuration->mqtt_broker, value, sizeof(retransmitter_configuration->mqtt_broker));
            } else if(strcmp(key, "mqtt_port") == 0) {
                retransmitter_configuration->mqtt_port = atoi(value);
            }
        }

        fclose(file);
    }
    else{
        syslog(LOG_INFO, "Error: Could not open configuration file %s (file does not exists or read not permitted)\n", configuration_file);
    }

    const char *env_channel = getenv(NRF24_CONFIGURATION_CHANNEL_ENV_NAME);
    if(env_channel != NULL) {
        retransmitter_configuration->channel = atoi(env_channel);
    }

    const char *env_rx_payload_size = getenv(NRF24_CONFIGURATION_RX_PAYLOAD_SIZE_ENV_NAME);
    if(env_rx_payload_size != NULL) {
        retransmitter_configuration->rx_payload_size = atoi(env_rx_payload_size);
    }

    const char *env_data_rate = getenv(NRF24_CONFIGURATION_DATA_RATE_ENV_NAME);
    if(env_data_rate != NULL) {
        retransmitter_configuration->data_rate = atoi(env_data_rate);
    }

    const char *env_mqtt_broker = getenv(MQTT_CONFIGURATION_BROKER_ENV_NAME);
    if(env_mqtt_broker != NULL) {
        strncpy(retransmitter_configuration->mqtt_broker, env_mqtt_broker, sizeof(retransmitter_configuration->mqtt_broker));
    }

    const char *env_mqtt_port = getenv(MQTT_CONFIGURATION_PORT_ENV_NAME);
    if(env_mqtt_port != NULL) {
        retransmitter_configuration->mqtt_port = atoi(env_mqtt_port);
    }
}

static void signal_handler(int sig) {
    switch(sig) {
        case SIGINT:
        case SIGTERM:
            closelog();
            exit(EXIT_SUCCESS);
            break;
        default:
            break;
    }
}