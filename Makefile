BASE_CC = gcc
CFLAGS ?= -g -Wall -Wextra -Werror
LDFLAGS ?= -lpthread -lrt
TARGET ?= retransmitter
CC ?= $(CROSS_COMPILE)$(BASE_CC)

all: $(TARGET)

retransmitter: src/main.c src/mqtt_module.c
	$(CC) $(CFLAGS) -I ./inc/ -I ./MQTT-C/include -o $(TARGET) src/main.c MQTT-C/src/mqtt_pal.c MQTT-C/src/mqtt.c src/mqtt_module.c $(LDFLAGS)
clean:
	rm -f $(TARGET)