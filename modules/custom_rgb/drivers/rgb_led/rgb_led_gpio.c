#define DT_DRV_COMPAT custom_rgb_led

#include <zephyr/kernel.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/sys/util.h>
#include <drivers/rgb_led.h>

struct rgb_led_config {
    struct gpio_dt_spec red_gpio;
    struct gpio_dt_spec green_gpio;
    struct gpio_dt_spec blue_gpio;
};

static int rgb_led_gpio_set_color(const struct device *dev, uint8_t r, uint8_t g, uint8_t b)
{
    const struct rgb_led_config *config = dev->config;
    uint8_t max_val = MAX(MAX(r, g), b);

    gpio_pin_set_dt(&config->red_gpio,   (r == max_val && r > 0) ? 1 : 0);
    gpio_pin_set_dt(&config->green_gpio, (g == max_val && g > 0) ? 1 : 0);
    gpio_pin_set_dt(&config->blue_gpio,  (b == max_val && b > 0) ? 1 : 0);

    return 0;
}

static int rgb_led_gpio_init(const struct device *dev)
{
    const struct rgb_led_config *config = dev->config;
    int ret;

    if (!gpio_is_ready_dt(&config->red_gpio) ||
        !gpio_is_ready_dt(&config->green_gpio) ||
        !gpio_is_ready_dt(&config->blue_gpio)) {
        return -ENODEV;
    }

    ret = gpio_pin_configure_dt(&config->red_gpio, GPIO_OUTPUT_INACTIVE);
    if (ret < 0) return ret;

    ret = gpio_pin_configure_dt(&config->green_gpio, GPIO_OUTPUT_INACTIVE);
    if (ret < 0) return ret;

    ret = gpio_pin_configure_dt(&config->blue_gpio, GPIO_OUTPUT_INACTIVE);
    if (ret < 0) return ret;

    return 0;
}

static const struct rgb_led_driver_api rgb_led_gpio_api = {
    .set_color = rgb_led_gpio_set_color,
};

#define RGB_LED_INIT(inst)                                                          \
    static const struct rgb_led_config rgb_led_config_##inst = {                    \
        .red_gpio   = GPIO_DT_SPEC_GET_BY_IDX(DT_DRV_INST(inst), gpios, 0),        \
        .green_gpio = GPIO_DT_SPEC_GET_BY_IDX(DT_DRV_INST(inst), gpios, 1),        \
        .blue_gpio  = GPIO_DT_SPEC_GET_BY_IDX(DT_DRV_INST(inst), gpios, 2),        \
    };                                                                              \
                                                                                    \
    DEVICE_DT_INST_DEFINE(inst,                                                     \
                          rgb_led_gpio_init,                                        \
                          NULL,                                                     \
                          NULL,                                                     \
                          &rgb_led_config_##inst,                                   \
                          POST_KERNEL,                                              \
                          CONFIG_KERNEL_INIT_PRIORITY_DEVICE,                       \
                          &rgb_led_gpio_api);

DT_INST_FOREACH_STATUS_OKAY(RGB_LED_INIT)