#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>
#include <zephyr/sys/util.h>
#include <zephyr/drivers/gpio.h>
#include "app_queues.h"
#include "system_one.h"

#define LED_RED_NODE DT_ALIAS(led_red)
#define LED_GREEN_NODE DT_ALIAS(led_green)
#define LED_BLUE_NODE DT_ALIAS(led_blue)

static const struct gpio_dt_spec led_red = GPIO_DT_SPEC_GET(LED_RED_NODE, gpios);
static const struct gpio_dt_spec led_green = GPIO_DT_SPEC_GET(LED_GREEN_NODE, gpios);
static const struct gpio_dt_spec led_blue = GPIO_DT_SPEC_GET(LED_BLUE_NODE, gpios);

void system_one_entry(void *p1, void *p2, void *p3) {
    ARG_UNUSED(p1);
    ARG_UNUSED(p2);
    ARG_UNUSED(p3);

    printk("[System One] System One thread started.\n");

    if(!gpio_is_ready_dt(&led_red) || !gpio_is_ready_dt(&led_green) || !gpio_is_ready_dt(&led_blue)) return;

    int ret_red = gpio_pin_configure_dt(&led_red, GPIO_OUTPUT_ACTIVE);
    if(ret_red < 0){
        printk("[System One] Failed to configure LED pin: %d\n", ret_red);
        return;
    }
    int red_green = gpio_pin_configure_dt(&led_green, GPIO_OUTPUT_ACTIVE);
    if(red_green < 0){
        printk("[System One] Failed to configure LED pin: %d\n", red_green);
        return;
    }
    int ret_blue = gpio_pin_configure_dt(&led_blue, GPIO_OUTPUT_ACTIVE);
    if(ret_blue < 0){
        printk("[System One] Failed to configure LED pin: %d\n", ret_blue);
        return;
    }

    struct rgb_cmd cmd;

    // turn all the leds off initially
    gpio_pin_set_dt(&led_red, 0);
    gpio_pin_set_dt(&led_green, 0);
    gpio_pin_set_dt(&led_blue, 0);

    while(1){
        // read from the RGB message queue
        if(k_msgq_get(&rgb_msgq, &cmd, K_NO_WAIT) == 0){
            printk("[System One] Received RGB command: R=%02X G=%02X B=%02X\n", cmd.red, cmd.green, cmd.blue);

            uint8_t max_value = MAX(MAX(cmd.red, cmd.green), cmd.blue);
            gpio_pin_set_dt(&led_red, cmd.red == max_value && cmd.red > 0 ? 1 : 0);
            gpio_pin_set_dt(&led_green, cmd.green == max_value && cmd.green > 0 ? 1 : 0);
            gpio_pin_set_dt(&led_blue, cmd.blue == max_value && cmd.blue > 0 ? 1 : 0);
        }
    }
}