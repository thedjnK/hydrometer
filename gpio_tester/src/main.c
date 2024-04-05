/*
 * Copyright (c) 2024 Jamie M.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/kernel.h>
#include <zephyr/device.h> 
#include <zephyr/drivers/gpio.h> 
#include <zephyr/sys/printk.h> 

/*
4, 5, 6, 7, 8, 15: 1.5v instead of 3.1v output
11, 12: dead short
18: 1.6v instead of 3.1v output
28, 29: mostly dead
31: 2.9v instead of 3.1v output
*/

#define GPIO_PIN_START 16
#define GPIO_PIN_END 18
#define SLEEP_TIME_MS 1000

int main(void)
{
	int rc;
	uint8_t i;
	const struct device *const gpio_dev = DEVICE_DT_GET(DT_NODELABEL(gpio0));

	k_sleep(K_MSEC(SLEEP_TIME_MS));

	i = GPIO_PIN_START;
	while (i < GPIO_PIN_END) {
		rc = gpio_pin_configure(gpio_dev, i, (GPIO_OUTPUT_INACTIVE | GPIO_ACTIVE_HIGH));

		if (rc) {
			printk("Configure failure: %d (%d)", rc, i);
		}

		++i;
	}

	i = GPIO_PIN_END;
	while (true) {
		rc = gpio_pin_set(gpio_dev, i, 0);

		if (rc) {
			printk("Set low failure: %d (%d)", rc, i);
		}

		if (i == GPIO_PIN_END) {
			i = GPIO_PIN_START;
		} else {
			++i;
		}

		rc = gpio_pin_set(gpio_dev, i, 1);

		if (rc) {
			printk("Set high failure: %d (%d)", rc, i);
		}

		k_sleep(K_MSEC(SLEEP_TIME_MS));
	}

	return 0;
}
