/*
 * Copyright (c) 2022 Carbon Aeronautics.
 *
 * Adapted from https://github.com/CarbonAeronautics/Part-XV-1DKalmanFilter/blob/main/ArduinoCode
 *
 * All right reserved.
 */

/* TODO: replace with sensorfusion */
#include <math.h>
#include <stdbool.h>
#include "filter.h"

static double kalman_angle_roll = 0;
static double kalman_uncertainty_angle_roll = 2 * 2;
static double kalman_angle_pitch = 0;
static double kalman_uncertainty_angle_pitch = 2 * 2;

static void kalman_1d(double *kalman_state, double *kalman_uncertainty, double kalman_input, double kalman_measurement) {
	*kalman_state = *kalman_state + 0.2 * kalman_input;
	*kalman_uncertainty = *kalman_uncertainty + 0.2 * 0.2 * 4 * 4;
	double kalman_gain = *kalman_uncertainty * 1 / (1 * *kalman_uncertainty + 3 * 3);
	*kalman_state = *kalman_state + kalman_gain * (kalman_measurement - *kalman_state);
	*kalman_uncertainty = (1 - kalman_gain) * *kalman_uncertainty;
}

void calculate_angle(const double *acceleration, const double *gyroscope, bool filter, double *roll, double *pitch)
{
	double angle_roll = atan(acceleration[1] / sqrt(pow(acceleration[0], 2) + pow(acceleration[2], 2))) * 180.0 / 3.141592653827;
	double angle_pitch = -atan(acceleration[0] / sqrt(pow(acceleration[1], 2) + pow(acceleration[2], 2))) * 180.0 / 3.141592653827;

	kalman_1d(&kalman_angle_roll, &kalman_uncertainty_angle_roll, gyroscope[0], angle_roll);
	kalman_1d(&kalman_angle_pitch, &kalman_uncertainty_angle_pitch, gyroscope[1], angle_pitch);

	*roll = kalman_angle_roll;
	*pitch = kalman_angle_pitch;

//      LOG_ERR("Got: %f, %f : %f, %f", angle_roll, angle_pitch, kalman_angle_roll, kalman_angle_pitch);
}
