#define DT_DRV_COMPAT custom_usart

#include <zephyr/device.h>
#include <zephyr/drivers/uart.h>
#include <errno.h>
#include <drivers/custom_usart.h>

struct custom_usart_config {
    const struct device *uart;
};

static int custom_usart_poll_in_impl(const struct device *dev, uint8_t *byte)
{
    const struct custom_usart_config *config = dev->config;

    return uart_poll_in(config->uart, byte);
}

static void custom_usart_poll_out_impl(const struct device *dev, unsigned char byte)
{
    const struct custom_usart_config *config = dev->config;

    uart_poll_out(config->uart, byte);
}

static int custom_usart_init(const struct device *dev)
{
    const struct custom_usart_config *config = dev->config;

    return device_is_ready(config->uart) ? 0 : -ENODEV;
}

static const struct custom_usart_driver_api custom_usart_api = {
    .poll_in = custom_usart_poll_in_impl,
    .poll_out = custom_usart_poll_out_impl,
};

#define CUSTOM_USART_INIT(inst)                                                   \
    static const struct custom_usart_config custom_usart_config_##inst = {         \
        .uart = DEVICE_DT_GET(DT_INST_PHANDLE(inst, uart)),                       \
    };                                                                             \
                                                                                   \
    DEVICE_DT_INST_DEFINE(inst,                                                    \
                          custom_usart_init,                                       \
                          NULL,                                                    \
                          NULL,                                                    \
                          &custom_usart_config_##inst,                            \
                          POST_KERNEL,                                             \
                          CONFIG_KERNEL_INIT_PRIORITY_DEVICE,                     \
                          &custom_usart_api);

DT_INST_FOREACH_STATUS_OKAY(CUSTOM_USART_INIT)