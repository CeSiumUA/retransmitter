#ifndef __NRF24_H__
#define __NRF24_H__

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
#include <fcntl.h>
#include <errno.h>

#define NRF24_DEVICE_PATH                                   "/dev/nrf24-0.%u"

#define NRF24_IOCTL_MAGIC                                   0x16

#define NRF24_IOCTL_SET_CHANNEL                             _IOW(NRF24_IOCTL_MAGIC, 0, uint8_t *)
#define NRF24_IOCTL_GET_CHANNEL                             _IOR(NRF24_IOCTL_MAGIC, 1, uint8_t *)

#define NRF24_IOCTL_SET_RX_PAYLOAD_SIZE                     _IOW(NRF24_IOCTL_MAGIC, 2, uint8_t *)
#define NRF24_IOCTL_GET_RX_PAYLOAD_SIZE                     _IOR(NRF24_IOCTL_MAGIC, 3, uint8_t *)

#define NRF24_IOCTL_SET_DATA_RATE                           _IOW(NRF24_IOCTL_MAGIC, 4, uint8_t *)
#define NRF24_IOCTL_GET_DATA_RATE                           _IOR(NRF24_IOCTL_MAGIC, 5, uint8_t *)

int nrf24_open(uint8_t channel, uint8_t rx_payload_size, uint8_t data_rate, uint8_t data_pipe);
void nrf24_close(void);
int nrf24_read(uint8_t *buffer, size_t buffer_size);
int nrf24_write(const uint8_t *buffer, size_t buffer_size);

#endif // __NRF24_H__