/*
 * Copyright (c) 2024 Jamie M.
 *
 * All right reserved. This code is not apache or FOSS/copyleft licensed.
 */

#include <zephyr/types.h>
#include <errno.h>
#include <zephyr/sys/byteorder.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/bluetooth/bluetooth.h>
#include <zephyr/bluetooth/conn.h>
#include <zephyr/bluetooth/uuid.h>
#include <zephyr/bluetooth/gatt.h>
//#include <zephyr/bluetooth/services/bas.h>
#include "bluetooth.h"

LOG_MODULE_DECLARE(app);

#define HYDROMETER_SVC_UUID BT_UUID_128_ENCODE(0x6565432a, 0xafc0, 0x4812, 0xbb2a, 0x22532a48b000)
#define CONTROL_CHAR_UUID BT_UUID_128_ENCODE(0x6565432a, 0xafc0, 0x4812, 0xbb2a, 0x22532a48b001)
#define TEMPERATURE_CHAR_UUID BT_UUID_128_ENCODE(0x6565432a, 0xafc0, 0x4812, 0xbb2a, 0x22532a48b002)
#define ROLL_CHAR_UUID BT_UUID_128_ENCODE(0x6565432a, 0xafc0, 0x4812, 0xbb2a, 0x22532a48b003)
#define PITCH_CHAR_UUID BT_UUID_128_ENCODE(0x6565432a, 0xafc0, 0x4812, 0xbb2a, 0x22532a48b004)

static const struct bt_uuid_128 hydrometer_svc_uuid = BT_UUID_INIT_128(HYDROMETER_SVC_UUID);
static const struct bt_uuid_128 hydrometer_control_char_uuid = BT_UUID_INIT_128(CONTROL_CHAR_UUID);
static const struct bt_uuid_128 temperature_reading_char_uuid = BT_UUID_INIT_128(TEMPERATURE_CHAR_UUID);
static const struct bt_uuid_128 roll_reading_char_uuid = BT_UUID_INIT_128(ROLL_CHAR_UUID);
static const struct bt_uuid_128 pitch_reading_char_uuid = BT_UUID_INIT_128(PITCH_CHAR_UUID);

static void bluetooth_advertise(void);

struct hydrometer_sensor_data {
	double temperature;
	double roll;
	double pitch;
};

static struct hydrometer_sensor_data sensor_data;

static void sensor_cccd_changed(const struct bt_gatt_attr *attr,
				 uint16_t value)
{
//	simulate_temp = value == BT_GATT_CCC_NOTIFY;
}

static ssize_t write_hyrdrometer_control(struct bt_conn *conn, const struct bt_gatt_attr *attr,
					 const void *buf, uint16_t len, uint16_t offset,
					 uint8_t flags)
{
	return 0;
}

static ssize_t read_sensor(struct bt_conn *conn, const struct bt_gatt_attr *attr, void *buf,
			   uint16_t len, uint16_t offset)
{
	const double *value = (const double *)attr->user_data;
	uint64_t send_value = sys_cpu_to_le64(*(uint64_t *)value);

	return bt_gatt_attr_read(conn, attr, buf, len, offset, &send_value, sizeof(send_value));
}

static void update_temperature(struct bt_conn *conn,
			       const struct bt_gatt_attr *chrc, int16_t value,
			       struct temperature_sensor *sensor)
{
#if 0
	/* Update temperature value */
	sensor->temp_value = value;

	/* Trigger notification if conditions are met */
	if (notify) {
		value = sys_cpu_to_le16(sensor->temp_value);

		bt_gatt_notify(conn, chrc, &value, sizeof(value));
	}
#endif
}

BT_GATT_SERVICE_DEFINE(hydrometer_svc,
	BT_GATT_PRIMARY_SERVICE(&hydrometer_svc_uuid),

	/* Control endpoint (clear settings/add calibration) */
	BT_GATT_CHARACTERISTIC(&hydrometer_control_char_uuid.uuid, BT_GATT_CHRC_WRITE,
			       BT_GATT_PERM_WRITE, NULL, write_hyrdrometer_control, NULL),

	/* Temperature reading */
	BT_GATT_CHARACTERISTIC(&temperature_reading_char_uuid.uuid,
			       (BT_GATT_CHRC_READ | BT_GATT_CHRC_NOTIFY), BT_GATT_PERM_READ,
			       read_sensor, NULL, &sensor_data.temperature),
	BT_GATT_CCC(sensor_cccd_changed, (BT_GATT_PERM_READ | BT_GATT_PERM_WRITE)),

	/* Roll reading */
	BT_GATT_CHARACTERISTIC(&roll_reading_char_uuid.uuid,
			       (BT_GATT_CHRC_READ | BT_GATT_CHRC_NOTIFY), BT_GATT_PERM_READ,
			       read_sensor, NULL, &sensor_data.roll),
	BT_GATT_CCC(sensor_cccd_changed, (BT_GATT_PERM_READ | BT_GATT_PERM_WRITE)),

	/* Pitch reading */
	BT_GATT_CHARACTERISTIC(&pitch_reading_char_uuid.uuid,
			       (BT_GATT_CHRC_READ | BT_GATT_CHRC_NOTIFY), BT_GATT_PERM_READ,
			       read_sensor, NULL, &sensor_data.pitch),
	BT_GATT_CCC(sensor_cccd_changed, (BT_GATT_PERM_READ | BT_GATT_PERM_WRITE)),

);

static const struct bt_data ad[] = {
	BT_DATA_BYTES(BT_DATA_FLAGS, (BT_LE_AD_GENERAL | BT_LE_AD_NO_BREDR)),
	BT_DATA_BYTES(BT_DATA_GAP_APPEARANCE, 0x00, 0x03),
	BT_DATA_BYTES(BT_DATA_UUID128_ALL, HYDROMETER_SVC_UUID),
};

static void bluetooth_connected(struct bt_conn *conn, uint8_t err)
{
	if (err) {
		LOG_INF("Connection failed: %d", err);
		bluetooth_advertise();
	} else {
		LOG_INF("Connected");
	}
}

static void bluetooth_disconnected(struct bt_conn *conn, uint8_t reason)
{
	printk("Disconnected: %d", reason);
	bluetooth_advertise();
}

BT_CONN_CB_DEFINE(conn_callbacks) = {
	.connected = bluetooth_connected,
	.disconnected = bluetooth_disconnected,
};

static void bluetooth_advertise(void)
{
	int rc;

	rc = bt_le_adv_start(BT_LE_ADV_CONN_NAME, ad, ARRAY_SIZE(ad), NULL, 0);

	if (rc) {
		LOG_ERR("Advertising failed to start: %d", rc);
		return;
	}

	LOG_INF("Advertising started");
}

#if 0
static void bas_notify(void)
{
	uint8_t battery_level = bt_bas_get_battery_level();

	battery_level--;

	if (!battery_level) {
		battery_level = 100U;
	}

	bt_bas_set_battery_level(battery_level);
}
#endif

int setup_bluetooth(void)
{
	int rc;

	rc = bt_enable(NULL);

	if (rc) {
		LOG_ERR("Bluetooth init failed: %d", rc);
		return rc;
	}

	bluetooth_advertise();

sensor_data.temperature = 123.456;
sensor_data.roll = 0.1;
sensor_data.pitch = 5.5;

	return 0;
}
