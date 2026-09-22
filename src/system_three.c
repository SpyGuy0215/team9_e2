#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <drivers/custom_piezo.h>
#include "app_queues.h"
#include "system_three.h"

static const struct device *const piezo_dev = DEVICE_DT_GET(DT_ALIAS(system_piezo));

void system_three_entry(void *p1, void *p2, void *p3)
{
	ARG_UNUSED(p1);
	ARG_UNUSED(p2);
	ARG_UNUSED(p3);

	if (!device_is_ready(piezo_dev)) {
		printk("[System Three] Error: Piezo device is not ready\n");
		return;
	}

	printk("[System Three] Startup complete with custom piezo DAC driver.\n");
	struct piezo_cmd cmd;

	while (1) {
		if (k_msgq_get(&piezo_msgq, &cmd, K_FOREVER) == 0) {
			printk("[System Three] Playing 660Hz tone for %u seconds...\n",
			       cmd.duration_seconds);
			custom_piezo_play(piezo_dev, 660, cmd.duration_seconds * 1000U);
		}
	}
}
