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
#include "mqtt.h"

int mqtt_module_init(const char * broker, const char * port);
void mqtt_module_deinit(void);

#endif // __MQTT_MODULE_H__