#define DT_DRV_COMPAT custom_piezo_dac

#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/logging/log.h>
#include <zephyr/drivers/pwm.h>
#include <drivers/custom_piezo.h>

LOG_MODULE_REGISTER(custom_piezo, CONFIG_LOG_DEFAULT_LEVEL);

struct custom_piezo_config {
    struct pwm_dt_spec pwm;
};

struct custom_piezo_data {
    struct k_work_delayable stop_work;
    const struct device *dev;
    bool is_playing;
};

static int custom_piezo_stop_impl(const struct device *dev)
{
    struct custom_piezo_data *data = dev->data;
    const struct custom_piezo_config *config = dev->config;

    if (!data->is_playing) {
        return 0;
    }

    int ret = pwm_set(config->pwm.dev, config->pwm.channel,
                      config->pwm.period, 0, config->pwm.flags);
    if (ret < 0) {
        LOG_ERR("Failed to stop piezo PWM: %d", ret);
        return ret;
    }

    data->is_playing = false;
    k_work_cancel_delayable(&data->stop_work);
    
    return 0;
}

static void stop_work_handler(struct k_work *work)
{
    struct k_work_delayable *dwork = k_work_delayable_from_work(work);
    struct custom_piezo_data *data = CONTAINER_OF(dwork, struct custom_piezo_data, stop_work);
    
    custom_piezo_stop_impl(data->dev);
}

static int custom_piezo_play_impl(const struct device *dev, uint32_t freq_hz, uint32_t duration_ms)
{
    struct custom_piezo_data *data = dev->data;
    const struct custom_piezo_config *config = dev->config;

    if (freq_hz == 0U || duration_ms == 0U) {
        return custom_piezo_stop_impl(dev);
    }

    uint32_t period_ns = PWM_NSEC(1000000000U / freq_hz);
    uint32_t pulse_ns = period_ns / 2U;
    int ret = pwm_set(config->pwm.dev, config->pwm.channel,
                      period_ns, pulse_ns, config->pwm.flags);
    if (ret < 0) {
        LOG_ERR("Failed to start piezo PWM: %d", ret);
        return ret;
    }

    data->is_playing = true;
    k_work_reschedule(&data->stop_work, K_MSEC(duration_ms));

    return 0;
}

static const struct custom_piezo_driver_api piezo_api = {
    .play = custom_piezo_play_impl,
    .stop = custom_piezo_stop_impl,
};

static int custom_piezo_init(const struct device *dev)
{
    struct custom_piezo_data *data = dev->data;
    const struct custom_piezo_config *config = dev->config;

    if (!pwm_is_ready_dt(&config->pwm)) {
        LOG_ERR("Piezo PWM device is not ready");
        return -ENODEV;
    }

    data->dev = dev;
    data->is_playing = false;
    
    k_work_init_delayable(&data->stop_work, stop_work_handler);
    
    return 0;
}

#define PIEZO_INIT(inst)                                                       \
    static struct custom_piezo_data piezo_data_##inst;                         \
    static const struct custom_piezo_config piezo_cfg_##inst = {               \
        .pwm = PWM_DT_SPEC_INST_GET(inst),                                     \
    };                                                                         \
    DEVICE_DT_INST_DEFINE(inst,                                                \
                          custom_piezo_init,                                   \
                          NULL,                                                \
                          &piezo_data_##inst,                                  \
                          &piezo_cfg_##inst,                                   \
                          POST_KERNEL,                                         \
                          CONFIG_KERNEL_INIT_PRIORITY_DEVICE,                  \
                          &piezo_api);

DT_INST_FOREACH_STATUS_OKAY(PIEZO_INIT)