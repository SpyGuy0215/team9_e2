#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <drivers/custom_usart.h>
#include <errno.h>
#include <stdlib.h>
#include "app_queues.h"
#include "system_four.h"

#define USART_NODE DT_NODELABEL(usart0)

static const struct device *const serial_usart = DEVICE_DT_GET(USART_NODE);

static bool is_system_four_value(const char *line)
{
    char *end;
    unsigned long value;

    if (*line == '\0') {
        return false;
    }

    for (const char *character = line; *character != '\0'; character++) {
        if (*character < '0' || *character > '9') {
            return false;
        }
    }

    errno = 0;
    value = strtoul(line, &end, 10);
    return errno == 0 && *end == '\0' && value <= 255;
}

void system_four_entry(void *p1, void *p2, void *p3) {
    ARG_UNUSED(p1);
    ARG_UNUSED(p2);
    ARG_UNUSED(p3);

    if (!device_is_ready(serial_usart)) {
        printk("[System Four] Custom USART is not ready.\n");
        return;
    }

    printk("[System Four] Serial receiver started.\n");

    struct serial_msg message = {0};
    size_t length = 0;

    while (1) {
        uint8_t byte;

        if (custom_usart_poll_in(serial_usart, &byte) != 0) {
            k_msleep(1);
            continue;
        }

        if (byte == '\r' || byte == '\n') {
            if (length == 0) {
                continue;
            }

            message.data[length] = '\0';
            if (is_system_four_value(message.data)) {
                printk("[System Four] Captured value: %s\n", message.data);
            } else if (k_msgq_put(&serial_msgq, &message, K_NO_WAIT) != 0) {
                printk("[System Four] Serial message queue full; dropping message.\n");
            }

            length = 0;
            message.data[0] = '\0';
        } else if (length < SERIAL_MESSAGE_SIZE - 1) {
            message.data[length++] = (char)byte;
        } else {
            length = 0;
            message.data[0] = '\0';
            printk("[System Four] Serial message too long; dropping message.\n");
        }
    }
}