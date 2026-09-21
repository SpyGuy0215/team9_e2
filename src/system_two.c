#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/sys/printk.h>
#include <drivers/servo_motor.h>
#include "app_queues.h"
#include "system_two.h"

#define SERVO_NODE DT_NODELABEL(servo0)

static const struct device *const servo = DEVICE_DT_GET(SERVO_NODE);

void system_two_entry(void *p1, void *p2, void *p3)
{
    ARG_UNUSED(p1);
    ARG_UNUSED(p2);
    ARG_UNUSED(p3);

    printk("[System Two] System Two thread started.\n");

    if (!device_is_ready(servo)) {
        printk("[System Two] Servo device is not ready.\n");
        return;
    }

    struct servo_cmd cmd;

    while (1) {
        if (k_msgq_get(&servo_msgq, &cmd, K_FOREVER) == 0) {
            int ret = servo_set_angle(servo, cmd.angle);

            if (ret < 0) {
                printk("[System Two] Failed to set servo angle %u: %d\n",
                       cmd.angle, ret);
            } else {
                printk("[System Two] Servo angle set to %u\n", cmd.angle);
            }
        }
    }
}