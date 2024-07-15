#ifndef __MAIN_H__
#define __MAIN_H__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <syslog.h>
#include <signal.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/ioctl.h>
#include <time.h>
#include <errno.h>
#include "mqtt_module.h"

#define CONFIGURATION_FILE_PATH                     "/etc/retransmitter/config.ini"
#define NRF24_CONFIGURATION_CHANNEL_DEFAULT          36
#define NRF24_CONFIGURATION_CHANNEL_ENV_NAME         "RETRANSMITTER_NRF24_CHANNEL"
#define NRF24_CONFIGURATION_RX_PAYLOAD_SIZE_DEFAULT  32
#define NRF24_CONFIGURATION_RX_PAYLOAD_SIZE_ENV_NAME "RETRANSMITTER_NRF24_RX_PAYLOAD_SIZE"
#define NRF24_CONFIGURATION_DATA_RATE_DEFAULT        0 //1 Mbps
#define NRF24_CONFIGURATION_DATA_RATE_ENV_NAME       "RETRANSMITTER_NRF24_DATA_RATE"

#define MQTT_CONFIGURATION_BROKER_DEFAULT             "localhost"
#define MQTT_CONFIGURATION_BROKER_ENV_NAME           "RETRANSMITTER_MQTT_BROKER"
#define MQTT_CONFIGURATION_PORT_DEFAULT               "1883"
#define MQTT_CONFIGURATION_PORT_ENV_NAME             "RETRANSMITTER_MQTT_PORT"

struct retransmitter_configuration_t {
    uint8_t channel;
    uint8_t rx_payload_size;
    uint8_t data_rate;
    char mqtt_broker[256];
    char mqtt_port[6];
};

#endif // __MAIN_H__