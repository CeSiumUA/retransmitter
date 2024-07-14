BASE_CC = gcc
CFLAGS ?= -g -Wall -Wextra
LDFLAGS ?= -lpthread -lrt
TARGET ?= retransmitter
CC ?= $(CROSS_COMPILE)$(BASE_CC)

all: $(TARGET)

retransmitter: src/main.c
	$(CC) $(CFLAGS) -I ./inc/ -o $(TARGET) src/main.c $(LDFLAGS)
clean:
	rm -f $(TARGET)