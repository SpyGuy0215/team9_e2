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

extern struct k_msgq rgb_msgq;
extern struct k_msgq servo_msgq;

#endif /* APP_QUEUES_H_ */