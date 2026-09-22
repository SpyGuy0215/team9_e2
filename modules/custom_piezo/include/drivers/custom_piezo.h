#ifndef ZEPHYR_INCLUDE_DRIVERS_CUSTOM_PIEZO_H_
#define ZEPHYR_INCLUDE_DRIVERS_CUSTOM_PIEZO_H_

#include <zephyr/device.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef int (*custom_piezo_play_t)(const struct device *dev, uint32_t freq_hz, uint32_t duration_ms);
typedef int (*custom_piezo_stop_t)(const struct device *dev);

__subsystem struct custom_piezo_driver_api {
    custom_piezo_play_t play;
    custom_piezo_stop_t stop;
};

static inline int custom_piezo_play(const struct device *dev, uint32_t freq_hz, uint32_t duration_ms) {
    const struct custom_piezo_driver_api *api = (const struct custom_piezo_driver_api *)dev->api;
    return api->play(dev, freq_hz, duration_ms);
}

static inline int custom_piezo_stop(const struct device *dev) {
    const struct custom_piezo_driver_api *api = (const struct custom_piezo_driver_api *)dev->api;
    return api->stop(dev);
}

#ifdef __cplusplus
}
#endif

#endif /* ZEPHYR_INCLUDE_DRIVERS_CUSTOM_PIEZO_H_ */