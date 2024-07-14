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

int mqtt_module_init(const char * broker, const char * port);
int mqtt_module_publish(const char * topic, const char * message);
void mqtt_module_deinit(void);

#endif // __MQTT_MODULE_H__