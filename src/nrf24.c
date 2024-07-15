#include "nrf24.h"

static FILE *nrf24_device = NULL;

int nrf24_open(uint8_t channel, uint8_t rx_payload_size, uint8_t data_rate, uint8_t data_pipe) {
    char device_path[sizeof(NRF24_DEVICE_PATH) + 3] = {0};
    snprintf(device_path, sizeof(device_path), NRF24_DEVICE_PATH, data_pipe);
    int res = 0;

    uint8_t saved_config;

    nrf24_device = fopen(device_path, "r+");
    if(nrf24_device == NULL) {
        syslog(LOG_ERR, "Error: Could not open device %s\n", device_path);
        return -1;
    }

    int fd = fileno(nrf24_device);

    int flags = fcntl(fd, F_GETFL, 0);
    if(flags < 0) {
        syslog(LOG_ERR, "Error: Could not get file flags\n");
        goto err_exit;
    }

    flags |= O_NONBLOCK;
    res = fcntl(fd, F_SETFL, flags);
    if(res < 0) {
        syslog(LOG_ERR, "Error: Could not set file flags\n");
        goto err_exit;
    }

    res = ioctl(fd, NRF24_IOCTL_GET_CHANNEL, &saved_config);
    if(res < 0) {
        syslog(LOG_ERR, "Error: Could not get channel\n");
        goto err_exit;
    }

    syslog(LOG_INFO, "Saved channel: %d\n", saved_config);

    res = ioctl(fd, NRF24_IOCTL_SET_CHANNEL, &channel);
    if(res < 0) {
        syslog(LOG_ERR, "Error: Could not set channel\n");
        goto err_exit;
    }

    syslog(LOG_INFO, "Channel set to: %d\n", channel);

    res = ioctl(fd, NRF24_IOCTL_SET_RX_PAYLOAD_SIZE, &rx_payload_size);
    if(res < 0) {
        syslog(LOG_ERR, "Error: Could not set rx payload size\n");
        goto err_exit;
    }

    syslog(LOG_INFO, "RX payload size set to: %d\n", rx_payload_size);

    res = ioctl(fd, NRF24_IOCTL_SET_DATA_RATE, &data_rate);
    if(res < 0) {
        syslog(LOG_ERR, "Error: Could not set data rate\n");
        goto err_exit;
    }

    syslog(LOG_INFO, "Data rate set to: %d\n", data_rate);

    return 0;

err_exit:
    fclose(nrf24_device);
    return -1;
}

int nrf24_read(uint8_t *buffer, size_t buffer_size) {
    int bytes_read = 0;

    bytes_read = fread(buffer, 1, buffer_size, nrf24_device);
    if(bytes_read < 0 && errno == EAGAIN){
        return 0;
    }

    if(bytes_read < 0) {
        syslog(LOG_ERR, "Error: Could not read from device\n");
    }

    return bytes_read;
}

int nrf24_write(const uint8_t *buffer, size_t buffer_size) {
    int bytes_written = 0;

    do {
        bytes_written = fwrite(buffer, 1, buffer_size, nrf24_device);
        usleep(100);
    } while(bytes_written < 0 && errno == EAGAIN);

    if(bytes_written < 0) {
        syslog(LOG_ERR, "Error: Could not write to device\n");
    }

    return bytes_written;
}

void nrf24_close(void){
    fclose(nrf24_device);
}