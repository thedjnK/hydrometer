/*
 * Copyright (c) 2022 Carbon Aeronautics.
 *
 * Adapted from https://github.com/CarbonAeronautics/Part-XV-1DKalmanFilter/blob/main/ArduinoCode
 *
 * All right reserved.
 */

/* TODO: replace with sensorfusion */
#include <stdint.h>

void calculate_angle(const double *acceleration, const double *gyroscope, bool filter, double *roll, double *pitch);
