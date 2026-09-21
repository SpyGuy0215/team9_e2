#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/pwm.h>
#include <zephyr/sys/printk.h>
#include "app_queues.h"
#include "system_two.h"

#define SERVO_PWM_NODE DT_NODELABEL(servo_pwm)

static const struct pwm_dt_spec servo = PWM_DT_SPEC_GET(SERVO_PWM_NODE);

static uint32_t servo_pulse_ns(uint16_t angle)
{
    return 1000000U + ((uint32_t)angle * 1000000U) / 180U;
}

void system_two_entry(void *p1, void *p2, void *p3) {
    ARG_UNUSED(p1);
    ARG_UNUSED(p2);
    ARG_UNUSED(p3);

    printk("[System Two] System Two thread started.\n");

    if (!pwm_is_ready_dt(&servo)) {
        printk("[System Two] Servo PWM device is not ready.\n");
        return;
    }

    struct servo_cmd cmd;

    // 20 ms (50Hz) period for standard hobby servo
    const uint32_t period_ns = PWM_NSEC(20000000);

    // 1.5 ms pulse for neutral position (90 degrees)
    if (pwm_set_dt(&servo, period_ns, PWM_NSEC(1500000)) < 0) {
        printk("[System Two] Failed to initialize servo PWM.\n");
        return;
    }

    while (1) {
        if (k_msgq_get(&servo_msgq, &cmd, K_FOREVER) == 0) {
            uint32_t pulse_ns = servo_pulse_ns(cmd.angle);
            int ret = pwm_set_dt(&servo, period_ns, pulse_ns);

            if (ret < 0) {
                printk("[System Two] Failed to set servo angle %u: %d\n",
                       cmd.angle, ret);
            } else {
                printk("[System Two] Servo angle set to %u\n", cmd.angle);
            }
        }
    }
}