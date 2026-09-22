#ifndef APP_QUEUES_H_
#define APP_QUEUES_H_

#include <zephyr/kernel.h>

struct rgb_cmd {
    uint8_t red;
    uint8_t green;
    uint8_t blue;
};

struct servo_cmd {
    uint16_t angle;
};

struct piezo_cmd {
    uint32_t duration_seconds;
};

#define SERIAL_MESSAGE_SIZE 128

struct serial_msg {
    char data[SERIAL_MESSAGE_SIZE];
};

extern struct k_msgq serial_msgq;
extern struct k_msgq rgb_msgq;
extern struct k_msgq servo_msgq;
extern struct k_msgq piezo_msgq;

#endif /* APP_QUEUES_H_ */