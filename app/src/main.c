/*
 * Copyright (c) 2024 Jamie M.
 *
 * All right reserved. This code is not apache or FOSS/copyleft licensed.
 */

#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/sensor.h>
#include <zephyr/logging/log.h>
#include "filter.h"
#include "bluetooth.h"

LOG_MODULE_REGISTER(app, CONFIG_APPLICATION_LOG_LEVEL);

static int sensor_reading(const struct device *dev, double *temperature, double *acceleration, double *gyroscope)
{
	struct sensor_value temperature_holding;
	struct sensor_value acceleration_holding[3];
	struct sensor_value gyroscope_holding[3];
	int rc;
	uint8_t i = 0;

	*temperature = 0.0f;

	while (i < 3) {
		acceleration[i] = 0.0f;
		gyroscope[i] = 0.0f;
		++i;
	}

	i = 0;

	while (i < CONFIG_APP_READINGS) {
		rc = sensor_sample_fetch(dev);

		if (rc) {
			LOG_ERR("Fetch readings failed: %d", rc);
			return rc;
		}

		if (i == 0) {
			rc = sensor_channel_get(dev, SENSOR_CHAN_DIE_TEMP, &temperature_holding);

			if (rc) {
				LOG_ERR("Get temperature reading failed: %d", rc);
				return rc;
			}

			*temperature += sensor_value_to_double(&temperature_holding);
		}

		rc = sensor_channel_get(dev, SENSOR_CHAN_ACCEL_XYZ, &acceleration_holding[3 * i]);

		if (rc) {
			LOG_ERR("Get acceleration reading failed: %d", rc);
			return rc;
		}

		rc = sensor_channel_get(dev, SENSOR_CHAN_GYRO_XYZ, &gyroscope_holding[3 * i]);

		if (rc) {
			LOG_ERR("Get gyroscope reading failed: %d", rc);
			return rc;
		}

		acceleration[0] += sensor_value_to_double(&acceleration_holding[0]);
		acceleration[1] += sensor_value_to_double(&acceleration_holding[1]);
		acceleration[2] += sensor_value_to_double(&acceleration_holding[2]);

		gyroscope[0] += sensor_value_to_double(&gyroscope_holding[0]);
		gyroscope[1] += sensor_value_to_double(&gyroscope_holding[1]);
		gyroscope[2] += sensor_value_to_double(&gyroscope_holding[2]);

		++i;
	}

#if CONFIG_APP_READINGS > 0
	i = 0;

	while (i < 3) {
		acceleration[i] /= CONFIG_APP_READINGS;
		gyroscope[i] /= CONFIG_APP_READINGS;

		++i;
	}
#endif

	return 0;
}

int main(void)
{
	const struct device *const mpu6500 = DEVICE_DT_GET_ONE(invensense_mpu6050);

	setup_bluetooth();

	if (!device_is_ready(mpu6500)) {
		LOG_ERR("Device init failed: %s", mpu6500->name);

		return 0;
	}

	while (1) {
		double temperature;
		double acceleration[3];
		double gyroscope[3];
		double roll;
		double pitch;
		int rc;

		rc = sensor_reading(mpu6500, &temperature, acceleration, gyroscope);

		if (rc != 0) {
			break;
		}

		calculate_angle(acceleration, gyroscope, true, &roll, &pitch);
		LOG_ERR("Got: %f, %f", roll, pitch);

//		k_sleep(K_SECONDS(1));
		k_sleep(K_MSEC(50));
	}

	return 0;
}
