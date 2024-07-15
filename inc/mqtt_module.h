#ifndef __MQTT_MODULE_H__
#define __MQTT_MODULE_H__

#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <syslog.h>
#include <fcntl.h>
#include <stdbool.h>
#include <pthread.h>
#include <unistd.h>
#include <stdlib.h>
#include "mqtt.h"

#define MQTT_MODULE_TEMPERATURE_TOPIC                   "retransmitter/temperature"
#define MQTT_MODULE_GET_TEMPERATURE_TOPIC               "retransmitter/get_temperature"
#define MQTT_MODULE_CLIENT_NAME                         "retransmitter_1"

int mqtt_module_init(const char * broker, const char * port, void (*callback)(const char * topic, const char * message));
int mqtt_module_publish(const char * topic, const char * message);
void mqtt_module_deinit(void);

#endif // __MQTT_MODULE_H__