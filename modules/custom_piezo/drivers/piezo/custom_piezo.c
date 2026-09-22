#define DT_DRV_COMPAT custom_piezo_dac

#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/logging/log.h>
#include <zephyr/drivers/dac.h>
#include <soc/soc_caps.h>
#include <soc/gpio_num.h>
#include <hal/clk_tree_ll.h>
#include <hal/dac_ll.h>
#include <hal/dac_types.h>
#include <hal/rtc_io_hal.h>
#include <hal/rtc_io_periph.h>
#include <drivers/custom_piezo.h>

LOG_MODULE_REGISTER(custom_piezo, CONFIG_LOG_DEFAULT_LEVEL);

struct custom_piezo_config {
    struct dac_dt_spec dac;
};

struct custom_piezo_data {
    struct k_work_delayable stop_work;
    const struct device *dev;
    bool is_playing;
};

static void custom_piezo_rtc_pad_init(dac_channel_t channel)
{
    RTCIO.pad_dac[channel].mux_sel = 0;
    RTCIO.pad_dac[channel].fun_sel = 0;
    RTCIO.pad_dac[channel].fun_ie = 0;
    RTCIO.pad_dac[channel].slp_sel = 0;
    RTCIO.pad_dac[channel].slp_ie = 0;
    RTCIO.pad_dac[channel].slp_oe = 0;
    RTCIO.pad_dac[channel].rue = 0;
    RTCIO.pad_dac[channel].rde = 0;
}

static int custom_piezo_stop_impl(const struct device *dev)
{
    struct custom_piezo_data *data = dev->data;
    const struct custom_piezo_config *config = dev->config;

    if (!data->is_playing) {
        return 0;
    }

    dac_channel_t channel = (dac_channel_t)config->dac.channel_id;
    dac_ll_cw_enable_channel(channel, false);
    dac_ll_cw_generator_disable();
    dac_ll_power_down(channel);
    clk_ll_rc_fast_disable();

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

    if (freq_hz < 130U) {
        LOG_ERR("Piezo frequency must be at least 130 Hz");
        return -EINVAL;
    }

    dac_channel_t channel = (dac_channel_t)config->dac.channel_id;
    gpio_num_t gpio_num = channel == DAC_CHAN_0 ? GPIO_NUM_25 : GPIO_NUM_26;
    int rtcio_num = rtc_io_num_map[gpio_num];

    if (rtcio_num < 0) {
        LOG_ERR("DAC GPIO %d is not an RTCIO pin", gpio_num);
        return -EINVAL;
    }

    rtcio_hal_function_select(rtcio_num, RTCIO_LL_FUNC_RTC);
    rtcio_hal_iomux_func_sel(rtcio_num, RTCIO_LL_PIN_FUNC);
    rtcio_hal_input_disable(rtcio_num);
    rtcio_hal_output_disable(rtcio_num);
    rtcio_hal_pullup_disable(rtcio_num);
    rtcio_hal_pulldown_disable(rtcio_num);

    clk_ll_rc_fast_enable();
    dac_ll_power_on(channel);
    custom_piezo_rtc_pad_init(channel);
    dac_ll_rtc_sync_by_adc(false);
    dac_ll_cw_set_freq(freq_hz, 8000000U);
    dac_ll_cw_set_atten(channel, DAC_COSINE_ATTEN_DB_0);
    dac_ll_cw_set_phase(channel, DAC_COSINE_PHASE_0);
    dac_ll_cw_set_dc_offset(channel, 0);
    dac_ll_cw_enable_channel(channel, true);
    dac_ll_cw_generator_enable();

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

    if (!device_is_ready(config->dac.dev)) {
        LOG_ERR("Piezo DAC device is not ready");
        return -ENODEV;
    }

    struct dac_channel_cfg channel_cfg = {
        .channel_id = config->dac.channel_id,
        .resolution = 8,
    };
    int ret = dac_channel_setup(config->dac.dev, &channel_cfg);
    if (ret < 0) {
        LOG_ERR("Failed to configure piezo DAC channel: %d", ret);
        return ret;
    }

    data->dev = dev;
    data->is_playing = false;
    
    k_work_init_delayable(&data->stop_work, stop_work_handler);
    
    return 0;
}

#define PIEZO_INIT(inst)                                                       \
    static struct custom_piezo_data piezo_data_##inst;                         \
    static const struct custom_piezo_config piezo_cfg_##inst = {               \
        .dac = DAC_DT_SPEC_INST_GET(inst),                                     \
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