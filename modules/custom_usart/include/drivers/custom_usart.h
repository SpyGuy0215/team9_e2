#ifndef ZEPHYR_INCLUDE_DRIVERS_CUSTOM_USART_H_
#define ZEPHYR_INCLUDE_DRIVERS_CUSTOM_USART_H_

#include <zephyr/device.h>
#include <zephyr/types.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef int (*custom_usart_poll_in_t)(const struct device *dev, uint8_t *byte);
typedef void (*custom_usart_poll_out_t)(const struct device *dev, unsigned char byte);

__subsystem struct custom_usart_driver_api {
    custom_usart_poll_in_t poll_in;
    custom_usart_poll_out_t poll_out;
};

static inline int custom_usart_poll_in(const struct device *dev, uint8_t *byte)
{
    const struct custom_usart_driver_api *api =
        (const struct custom_usart_driver_api *)dev->api;

    if (api == NULL || api->poll_in == NULL) {
        return -ENOSYS;
    }

    return api->poll_in(dev, byte);
}

static inline void custom_usart_poll_out(const struct device *dev, unsigned char byte)
{
    const struct custom_usart_driver_api *api =
        (const struct custom_usart_driver_api *)dev->api;

    if (api == NULL || api->poll_out == NULL) {
        return;
    }

    return api->poll_out(dev, byte);
}

#ifdef __cplusplus
}
#endif

#endif /* ZEPHYR_INCLUDE_DRIVERS_CUSTOM_USART_H_ */