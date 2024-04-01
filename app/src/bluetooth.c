/*
 * Copyright (c) 2024 Jamie M.
 *
 * All right reserved. This code is not apache or FOSS/copyleft licensed.
 */

//#include <stdbool.h>
#include <zephyr/types.h>
//#include <stddef.h>
//#include <string.h>
#include <errno.h>
//#include <zephyr/sys/printk.h>
#include <zephyr/sys/byteorder.h>
#include <zephyr/kernel.h>

#include <zephyr/bluetooth/bluetooth.h>
//#include <zephyr/bluetooth/hci.h>
#include <zephyr/bluetooth/conn.h>
#include <zephyr/bluetooth/uuid.h>
#include <zephyr/bluetooth/gatt.h>
#include <zephyr/bluetooth/services/bas.h>

#include "bluetooth.h"

#define HYDROMETER_SVC_UUID BT_UUID_128_ENCODE(0x6565432a, 0xafc0, 0x4812, 0xbb2a, 0x22532a48b000)

static const struct bt_uuid_128 hydrometer_svc_uuid = BT_UUID_INIT_128(HYDROMETER_SVC_UUID);

static const struct bt_uuid_128 hydrometer_control_char_uuid = BT_UUID_INIT_128(
        0x65, 0x65, 0x43, 0x2a, 0xaf, 0xc0, 0x48, 0x12,
	0xbb, 0x2a, 0x22, 0x53, 0x2a, 0x48, 0xb0, 0x01);

static const struct bt_uuid_128 hydrometer_reading_char_uuid = BT_UUID_INIT_128(
        0x65, 0x65, 0x43, 0x2a, 0xaf, 0xc0, 0x48, 0x12,
	0xbb, 0x2a, 0x22, 0x53, 0x2a, 0x48, 0xb0, 0x02);

static ssize_t write_hyrdrometer_control(struct bt_conn *conn, const struct bt_gatt_attr *attr,
					 const void *buf, uint16_t len, uint16_t offset,
					 uint8_t flags)
{
	return 0;
}

static ssize_t read_u16(struct bt_conn *conn, const struct bt_gatt_attr *attr,
			void *buf, uint16_t len, uint16_t offset)
{
	const uint16_t *u16 = attr->user_data;
	uint16_t value = sys_cpu_to_le16(*u16);

	return bt_gatt_attr_read(conn, attr, buf, len, offset, &value,
				 sizeof(value));
}

/* Environmental Sensing Service Declaration */

//struct hydrometer_sensor {
//	double temperature;
//	double roll;
//	double pitch;
//};

//static bool simulate_temp;

static void temp_ccc_cfg_changed(const struct bt_gatt_attr *attr,
				 uint16_t value)
{
//	simulate_temp = value == BT_GATT_CCC_NOTIFY;
}

static ssize_t read_es_measurement(struct bt_conn *conn,
				   const struct bt_gatt_attr *attr, void *buf,
				   uint16_t len, uint16_t offset)
{
/*	const struct es_measurement *value = attr->user_data;
	struct read_es_measurement_rp rsp;

	rsp.flags = sys_cpu_to_le16(value->flags);
	rsp.sampling_function = value->sampling_func;
	sys_put_le24(value->meas_period, rsp.measurement_period);
	sys_put_le24(value->update_interval, rsp.update_interval);
	rsp.application = value->application;
	rsp.measurement_uncertainty = value->meas_uncertainty;

	return bt_gatt_attr_read(conn, attr, buf, len, offset, &rsp,
				 sizeof(rsp));*/
return 0;
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

	/* Get reading */
	BT_GATT_CHARACTERISTIC(&hydrometer_reading_char_uuid.uuid,
			       BT_GATT_CHRC_READ | BT_GATT_CHRC_NOTIFY,
			       BT_GATT_PERM_READ,
			       read_u16, NULL, 
NULL),
//&sensor_1.temp_value),

	BT_GATT_CCC(temp_ccc_cfg_changed,
		    BT_GATT_PERM_READ | BT_GATT_PERM_WRITE),

);

static const struct bt_data ad[] = {
	BT_DATA_BYTES(BT_DATA_FLAGS, (BT_LE_AD_GENERAL | BT_LE_AD_NO_BREDR)),
	BT_DATA_BYTES(BT_DATA_GAP_APPEARANCE, 0x00, 0x03),
	BT_DATA_BYTES(BT_DATA_UUID128_ALL, HYDROMETER_SVC_UUID),
};

static void connected(struct bt_conn *conn, uint8_t err)
{
	if (err) {
		printk("Connection failed (err 0x%02x)\n", err);
	} else {
		printk("Connected\n");
	}
}

static void disconnected(struct bt_conn *conn, uint8_t reason)
{
	printk("Disconnected (reason 0x%02x)\n", reason);
}

BT_CONN_CB_DEFINE(conn_callbacks) = {
	.connected = connected,
	.disconnected = disconnected,
};

static void bt_ready(void)
{
	int err;

	printk("Bluetooth initialized\n");

	err = bt_le_adv_start(BT_LE_ADV_CONN_NAME, ad, ARRAY_SIZE(ad), NULL, 0);
	if (err) {
		printk("Advertising failed to start (err %d)\n", err);
		return;
	}

	printk("Advertising successfully started\n");
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
	int err;

	err = bt_enable(NULL);

	if (err) {
		printk("Bluetooth init failed (err %d)\n", err);
		return err;
	}

	bt_ready();

//	while (1) {
//		k_sleep(K_SECONDS(1));
//
//		/* Battery level simulation */
//		bas_notify();
//	}

	return 0;
}
