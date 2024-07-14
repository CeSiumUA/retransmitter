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

#define CONFIGURATION_FILE_PATH                     "/etc/retransmitter/config.ini"
#define NRF24_CONFIGURATION_CHANNEL_DEFAULT          36
#define NRF24_CONFIGURATION_CHANNEL_ENV_NAME         "NRF24_CHANNEL"
#define NRF24_CONFIGURATION_RX_PAYLOAD_SIZE_DEFAULT  32
#define NRF24_CONFIGURATION_RX_PAYLOAD_SIZE_ENV_NAME "NRF24_RX_PAYLOAD_SIZE"
#define NRF24_CONFIGURATION_DATA_RATE_DEFAULT        0 //1 Mbps
#define NRF24_CONFIGURATION_DATA_RATE_ENV_NAME       "NRF24_DATA_RATE"

struct nrf24_configuration_t {
    uint8_t channel;
    uint8_t rx_payload_size;
    uint8_t data_rate;
};

#endif // __MAIN_H__