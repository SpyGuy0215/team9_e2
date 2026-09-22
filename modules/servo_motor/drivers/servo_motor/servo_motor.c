#define DT_DRV_COMPAT custom_servo_motor

#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/pwm.h>
#include <zephyr/logging/log.h>
#include <zephyr/sys/printk.h>
#include <drivers/servo_motor.h>

LOG_MODULE_REGISTER(servo_motor, CONFIG_LOG_DEFAULT_LEVEL);

struct servo_motor_config {
    struct pwm_dt_spec pwm;
    uint32_t min_pulse_ns;
    uint32_t max_pulse_ns;
};

static int servo_motor_set_angle(const struct device *dev, uint16_t angle)
{
    const struct servo_motor_config *config = dev->config;

    if (angle > 180) {
        LOG_ERR("Angle %u out of bounds (0-180)", angle);
        return -EINVAL;
    }

    uint32_t pulse_range_ns = config->max_pulse_ns - config->min_pulse_ns;
    uint32_t pulse_ns = config->min_pulse_ns + ((uint32_t)angle * pulse_range_ns) / 180U;

    int ret = pwm_set_pulse_dt(&config->pwm, pulse_ns);
    if (ret < 0) {
        LOG_ERR("Failed to set PWM pulse: %d", ret);
        return ret;
    }

    return 0;
}

static int servo_motor_init(const struct device *dev)
{
    const struct servo_motor_config *config = dev->config;

    if (!pwm_is_ready_dt(&config->pwm)) {
        LOG_ERR("PWM device %s is not ready", config->pwm.dev->name);
        printk("[Servo] PWM device %s is not ready.\n", config->pwm.dev->name);
        return -ENODEV;
    }

    /* Set default neutral position (90 degrees) on boot */
    int ret = servo_motor_set_angle(dev, 90);
    if (ret < 0) {
        printk("[Servo] Initial PWM setup failed: %d.\n", ret);
    }

    return ret;
}

static const struct servo_motor_driver_api servo_motor_api = {
    .set_angle = servo_motor_set_angle,
};

#define SERVO_MOTOR_INIT(inst)                                                   \
    static const struct servo_motor_config servo_motor_config_##inst = {        \
        .pwm = PWM_DT_SPEC_INST_GET(inst),                                       \
        .min_pulse_ns = DT_INST_PROP_OR(inst, min_pulse_us, 1000) * 1000U,        \
        .max_pulse_ns = DT_INST_PROP_OR(inst, max_pulse_us, 2000) * 1000U,        \
    };                                                                            \
                                                                                  \
    DEVICE_DT_INST_DEFINE(inst,                                                   \
                         servo_motor_init,                                        \
                         NULL,                                                    \
                         NULL,                                                    \
                         &servo_motor_config_##inst,                              \
                         POST_KERNEL,                                             \
                         CONFIG_KERNEL_INIT_PRIORITY_DEVICE,                      \
                         &servo_motor_api);

DT_INST_FOREACH_STATUS_OKAY(SERVO_MOTOR_INIT)