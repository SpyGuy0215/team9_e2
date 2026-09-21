#ifndef ZEPHYR_INCLUDE_DRIVERS_RGB_LED_H_
#define ZEPHYR_INCLUDE_DRIVERS_RGB_LED_H_

#include <zephyr/device.h>
#include <zephyr/types.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef int (*rgb_led_api_set_color_t)(const struct device *dev, uint8_t r, uint8_t g, uint8_t b);

__subsystem struct rgb_led_driver_api {
    rgb_led_api_set_color_t set_color;
};

/**
 * @brief Set the RGB output based on RGB intensity values.
 */
static inline int rgb_led_set_color(const struct device *dev, uint8_t r, uint8_t g, uint8_t b)
{
    const struct rgb_led_driver_api *api =
        (const struct rgb_led_driver_api *)dev->api;

    if (api == NULL || api->set_color == NULL) {
        return -ENOSYS;
    }

    return api->set_color(dev, r, g, b);
}

#ifdef __cplusplus
}
#endif

#endif /* ZEPHYR_INCLUDE_DRIVERS_RGB_LED_H_ */