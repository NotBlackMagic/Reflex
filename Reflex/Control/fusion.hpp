/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright (c) 2026 NotBlackMagic (PlumaLabs)
 *
 * File:    Instinct/Modules/Control/fusion.hpp
 * Author:  NotBlackMagic
 * Brief:   Simple complementary filter for state estimation.
 */

#pragma once

#include <cstdint>

class SensorFusion {
	public:
		static void Init();

		/// @brief Updates the sensor fusion filter. Call this at exactly 1kHz.
		/// @param accel	Raw MPU6050 accelerometer data (X, Y, Z)
		/// @param gyro		Raw MPU6050 gyroscope data (X, Y, Z)
		/// @return None
		static void Update(const int16_t accel[3], const int16_t gyro[3]);

		/// @brief Get pitch rotation component in 0.01 deg/LSB
		/// @return Gimbal Pitch in 0.01 degree units (e.g., 4500 = 45.00 deg)
		static int32_t GetPitch() { return pitch; }

		/// @brief Get roll rotation component in 0.01 deg/LSB
		/// @return Gimbal Roll in 0.01 degree units (e.g., -1500 = -15.00 deg)
		static int32_t GetRoll() { return roll; }

		/// @brief Get yaw rotation component in 0.01 deg/LSB
		/// @return Gimbal Yaw in 0.01 degree units (Unreferenced integration)
		static int32_t GetYaw() { return yaw; }

	private:
		static int32_t pitch;
		static int32_t roll;
		static int32_t yaw;

		// Fast integer approximation of atan2(y, x) returning 0.01 degrees
		static int32_t FastAtan2(int32_t y, int32_t x);
};