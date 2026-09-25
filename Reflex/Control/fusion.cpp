/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright (c) 2026 NotBlackMagic (PlumaLabs)
 *
 * File:    Instinct/Modules/Control/fusion.cpp
 */

#include "fusion.hpp"

// Static allocations
int32_t SensorFusion::pitch = 0;
int32_t SensorFusion::roll = 0;
int32_t SensorFusion::yaw = 0;

void SensorFusion::Init() {
	pitch = 0;
	roll = 0;
	yaw = 0;
}

int32_t SensorFusion::FastAtan2(int32_t y, int32_t x) {
	if(x == 0 && y == 0) {
		return 0;
	}

	// Shift down to prevent 32-bit overflow during squaring/multiplication.
	int32_t sx = x >> 6; 
	int32_t sy = y >> 6;

	int32_t abs_x = sx >= 0 ? sx : -sx;
	int32_t abs_y = sy >= 0 ? sy : -sy;

	int32_t angle;
	int32_t num = 5729 * abs_y * abs_x; // 5729 = 180 / PI * 100
	int32_t den;

	if(abs_y <= abs_x) {
		// angle = (5729 * y * x) / (x^2 + 0.28 * y^2)
		den = (abs_x * abs_x) + ((abs_y * abs_y * 28) / 100);
		angle = den == 0 ? 0 : (num / den);
	}
	else {
		// angle = 90 - atan(x/y)
		den = (abs_y * abs_y) + ((abs_x * abs_x * 28) / 100);
		angle = den == 0 ? 0 : 9000 - (num / den);
	}

	if(sx < 0) {
		angle = 18000 - angle;
	}
	if(sy < 0) {
		angle = -angle;
	}

	return angle;
}

void SensorFusion::Update(const int16_t accel[3], const int16_t gyro[3]) {
	// Gyro Integration: 
	// IMU configured for 1000 DPS. 1 LSB = 0.030517 deg/s.
	// At 1kHz (1ms dt), the change is 0.000030517 degrees per LSB.
	// In our 0.01 degree units, delta = gyro * 0.0030517.
	// Magic fraction: 25 / 8192 is exactly 0.00305175!
	// This allows perfect integration using just an integer multiply and bit shift (>> 13).

	int32_t delta_pitch = (gyro[0] * 25) >> 13;
	int32_t delta_roll = (gyro[1] * 25) >> 13;
	int32_t delta_yaw = (gyro[2] * 25) >> 13;

	pitch += delta_pitch;
	roll += delta_roll;
	yaw += delta_yaw; // Yaw has no absolute gravity reference, it will slowly drift.

	// Accelerometer Absolute Angles
	int32_t accel_pitch = FastAtan2(-accel[0], accel[2]); 
	int32_t accel_roll = FastAtan2(accel[1], accel[2]);

	// Complementary Filter (Alpha = 0.996)
	// Formula: Angle = Angle * 0.996 + Accel * 0.004
	// Integer equivalent: Angle = (Angle * 255 + Accel) / 256

	pitch = (pitch * 255 + accel_pitch) >> 8;
	roll = (roll * 255 + accel_roll) >> 8;
}