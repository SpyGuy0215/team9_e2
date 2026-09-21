#include <zephyr/kernel.h>
#include <zephyr/drivers/gpio.h>
#include "system_zero.h"

#define LED0_NODE DT_ALIAS(led0)
static const struct gpio_dt_spec led0 = GPIO_DT_SPEC_GET(LED0_NODE, gpios);

void system_zero_entry(void *p1, void *p2, void *p3) {
    ARG_UNUSED(p1);
    ARG_UNUSED(p2);
    ARG_UNUSED(p3);

    if(!gpio_is_ready_dt(&led0)) return;

    int ret = gpio_pin_configure_dt(&led0, GPIO_OUTPUT_ACTIVE); // equiv to pinMode(LED_PIN, OUTPUT);
    if(ret < 0) return;

    while(1){
        gpio_pin_toggle_dt(&led0); // equiv to digitalWrite(LED_PIN, !digitalRead(LED_PIN));
        k_msleep(250);
    }
}