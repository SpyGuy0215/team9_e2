#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>
#include <zephyr/sys/util.h>
#include <drivers/rgb_led.h>    // custom rgb led driver api
#include "app_queues.h"
#include "system_one.h"

#define RGB_LED_NODE DT_NODELABEL(rgb_led0)

static const struct device *const rgb_led = DEVICE_DT_GET(RGB_LED_NODE);

void system_one_entry(void *p1, void *p2, void *p3) {
    ARG_UNUSED(p1);
    ARG_UNUSED(p2);
    ARG_UNUSED(p3);

    printk("[System One] System One thread started with custom rgb_led driver.\n");

    if (!device_is_ready(rgb_led)) {
        printk("[System One] RGB LED device not ready.\n");
        return;
    }

    struct rgb_cmd cmd;

    // turn all the leds off initially with the new cmd
    rgb_led_set_color(rgb_led, 0, 0, 0);

    while (1) {
        // read from the RGB message queue
        if (k_msgq_get(&rgb_msgq, &cmd, K_NO_WAIT) == 0) {
            printk("[System One] Received RGB command: R=%02X G=%02X B=%02X\n", cmd.red, cmd.green, cmd.blue);

            // driver function handles the on/off based on the hex values
            // so this code just passes them directly
            rgb_led_set_color(rgb_led, cmd.red, cmd.green, cmd.blue);
        }
    }
}