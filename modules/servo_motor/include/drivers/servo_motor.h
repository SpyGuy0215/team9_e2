#ifndef ZEPHYR_INCLUDE_DRIVERS_SERVO_MOTOR_H_
#define ZEPHYR_INCLUDE_DRIVERS_SERVO_MOTOR_H_

#include <zephyr/device.h>
#include <errno.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Driver API structure.
 */
typedef int (*servo_set_angle_t)(const struct device *dev, uint16_t angle);

__subsystem struct servo_motor_driver_api {
    servo_set_angle_t set_angle;
};

/**
 * @brief Set the shaft angle of the servo motor.
 *
 * @param dev Pointer to the servo motor device structure.
 * @param angle Target angle in degrees (0 to 180).
 *
 * @return 0 on success, negative error code on failure.
 */
static inline int servo_set_angle(const struct device *dev, uint16_t angle)
{
    const struct servo_motor_driver_api *api =
        (const struct servo_motor_driver_api *)dev->api;

    if (api == NULL || api->set_angle == NULL) {
        return -ENOSYS;
    }

    return api->set_angle(dev, angle);
}

#ifdef __cplusplus
}
#endif

#endif /* ZEPHYR_INCLUDE_DRIVERS_SERVO_MOTOR_H_ */