/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright (c) 2026 NotBlackMagic (PlumaLabs)
 *
 * File:    SDK/Drivers/Sensors/mpu6050.hpp
 * Author:  NotBlackMagic
 * Brief:   MPU-6050 6-Axis IMU driver class for STM32N6.
 */

#pragma once

#include <stdint.h>

#include "i2c.hpp"
#include "status.hpp"

class MPU6050 {
	public:
		// Standard Chip identifications
		static constexpr uint8_t i2cAddrPrimary = 0x68;
		static constexpr uint8_t i2cAddrSecondary = 0x69;
		static constexpr uint8_t chipID = 0x68;

		/// @brief Device register map.
		enum class Register : uint8_t {
			SELF_TEST_X = 0x0D,
			SELF_TEST_Y = 0x0E,
			SELF_TEXT_Z = 0x0F,
			SELF_TEST_A = 0x10,
			SMPRT_DIV = 0x19,
			MPU_CONFIG = 0x1A,
			GYRO_CONFIG = 0x1B,
			ACCEL_CONFIG = 0x1C,
			MOT_THR = 0x1F,
			FIFO_EN = 0x23,
			I2C_MST_CTRL = 0x24,
			I2C_SLV0_ADDR = 0x25,
			I2C_SLV0_REG = 0x26,
			I2C_SLV0_CTRL = 0x27,
			I2C_SLV1_ADDR = 0x28,
			I2C_SLV1_REG = 0x29,
			I2C_SLV1_CTRL = 0x2A,
			I2C_SLV2_ADDR = 0x2B,
			I2C_SLV2_REG = 0x2C,
			I2C_SLV2_CTRL = 0x2D,
			I2C_SLV3_ADDR = 0x2E,
			I2C_SLV3_REG = 0x2F,
			I2C_SLV3_CTRL = 0x30,
			I2C_SLV4_ADDR = 0x31,
			I2C_SLV4_REG = 0x32,
			I2C_SLV4_DO = 0x33,
			I2C_SLV4_CTRL = 0x34,
			I2C_SLV4_DI = 0x35,
			I2C_MST_STATUS = 0x36,
			INT_PIN_CFG = 0x37,
			INT_ENABLE = 0x38,
			INT_STATUS = 0x3A,
			ACCEL_XOUT_H = 0x3B,
			ACCEL_XOUT_L = 0x3C,
			ACCEL_YOUT_H = 0x3D,
			ACCEL_YOUT_L = 0x3E,
			ACCEL_ZOUT_H = 0x3F,
			ACCEL_ZOUT_L = 0x40,
			TEMP_OUT_H = 0x41,
			TEMP_OUT_L = 0x42,
			GYRO_XOUT_H = 0x43,
			GYRO_XOUT_L = 0x44,
			GYRO_YOUT_H = 0x45,
			GYRO_YOUT_L = 0x46,
			GYRO_ZOUT_H = 0x47,
			GYRO_ZOUT_L = 0x48,
			EXT_SENS_DATA_00 = 0x49,
			EXT_SENS_DATA_01 = 0x4A,
			EXT_SENS_DATA_02 = 0x4B,
			EXT_SENS_DATA_03 = 0x4C,
			EXT_SENS_DATA_04 = 0x4D,
			EXT_SENS_DATA_05 = 0x4E,
			EXT_SENS_DATA_06 = 0x4F,
			EXT_SENS_DATA_07 = 0x50,
			EXT_SENS_DATA_08 = 0x51,
			EXT_SENS_DATA_09 = 0x52,
			EXT_SENS_DATA_10 = 0x53,
			EXT_SENS_DATA_11 = 0x54,
			EXT_SENS_DATA_12 = 0x55,
			EXT_SENS_DATA_13 = 0x56,
			EXT_SENS_DATA_14 = 0x57,
			EXT_SENS_DATA_15 = 0x58,
			EXT_SENS_DATA_16 = 0x59,
			EXT_SENS_DATA_17 = 0x5A,
			EXT_SENS_DATA_18 = 0x5B,
			EXT_SENS_DATA_19 = 0x5C,
			EXT_SENS_DATA_20 = 0x5D,
			EXT_SENS_DATA_21 = 0x5E,
			EXT_SENS_DATA_22 = 0x5F,
			EXT_SENS_DATA_23 = 0x60,
			I2C_SLV0_DO = 0x63,
			I2C_SLV1_DO = 0x64,
			I2C_SLV2_DO = 0x65,
			I2C_SLV3_DO = 0x66,
			I2C_MST_DELAY_CTRL = 0x67,
			SIGNAL_PATH_RESET = 0x68,
			MOT_DETECT_CTRL = 0x69,
			USER_CTRL = 0x6A,
			PWR_MGMT_1 = 0x6B,
			PWR_MGMT_2 = 0x6C,
			FIFO_COUNTH = 0x72,
			FIFO_COUNTL = 0x73,
			FIFO_R_W = 0x74,
			WHO_AM_I = 0x75
		};

		/// @brief Supported output data rates (ODR).
		enum class OutputDataRate : uint8_t {
			Hz6400 = 0x03,
			Hz3200 = 0x04,
			Hz1600 = 0x05,
			Hz800 = 0x06,
			Hz400 = 0x07,
			Hz200 = 0x08,
			Hz100 = 0x09,
			Hz50 = 0x0A,
			Hz25 = 0x0B,
			Hz12_5 = 0x0C,
			Hz6_25 = 0x0D,
			Hz3_125 = 0x0E,
			Hz1_5625 = 0x0F
		};

		/// @brief Supported accelerometer output full scale (FS).
		enum class AccelScale : uint8_t {
			G2 = 0x00,
			G4 = 0x01,
			G8 = 0x02,
			G16 = 0x03			
		};

		/// @brief Supported gyroscope output full scale (FS).
		enum class GyroScale : uint8_t {
			DPS250 = 0x00,
			DPS500 = 0x01,
			DPS1000 = 0x02,
			DPS2000 = 0x03			
		};

		/// @brief MPU-6050 sensor configuration structure.
		struct Config {
			AccelScale accelScale;
			GyroScale gyroScale;
			OutputDataRate accelOdr;
			OutputDataRate gyroOdr;
		};

		/// @brief Constructor.
		/// @param i2c	Reference to the low-level bus driver.
		/// @param addr	Bus address.
		MPU6050(I2C& i2c, uint8_t addr) : bus(i2c), addr(addr) {};

		/// @brief Initializes the MPU-6050 power monitor.
		/// @param config MPU-6050 configuration.
		/// @return Status::Ok if initialization succeeded, or Status::Error if the config was invalid or failed.
		Status Init(const Config& config);

		/// @brief Resets the sensor device, using software reset.
		/// @return Status::Ok if reset succeeded cleanly.
		Status Reset();

		/// @brief Reads the Manufacturer and Device IDs.
		/// @param id Device ID, or manufacturer ID.
		/// @return Status::Ok if read succeeded, or Status::Error if failed.
		Status ReadID(uint8_t& id);

		/// @brief Sets the sensors data output full scale.
		/// @param accelScale	Accelerometer output full scale.
		/// @param gyroScale	Gyroscope output full scale.
		/// @return Status::Ok if set succeeded, or Status::Error if failed.
		Status SetScales(AccelScale accelScale, GyroScale gyroScale);

		/// @brief Sets the accelerometer offset value, to be added to the read value.
		/// @param offsetX Offset for x-axis.
		/// @param offsetY Offset for y-axis.
		/// @param offsetZ Offset for z-axis.
		/// @return Status::Ok if set succeeded, or Status::Error if failed.
		Status SetAccelOffsets(float offsetX, float offsetY, float offsetZ);

		/// @brief Sets the gyroscope offset value, to be added to the read value.
		/// @param offsetX Offset for x-axis.
		/// @param offsetY Offset for y-axis.
		/// @param offsetZ Offset for z-axis.
		/// @return Status::Ok if set succeeded, or Status::Error if failed.
		Status SetGyroOffsets(float offsetX, float offsetY, float offsetZ);

		/// @brief Performs a hardware self test.
		/// @return Status::Ok if self test passed, Status::Error if failed.
		Status RunHardwareSelfTest();

		/// @brief Start a non-blocking data transfer request (calls TransferAsync of the underlying bus).
		/// @details Returns immediately. Use TransferWait() to synchronize completion.
		/// @return Status::Ok if the transfer started, or Status::Busy if the bus is locked by another thread.
		Status RequestData();

		/// @brief Parses the internal buffer into raw 16-bit integers (No Floats!)
		void ParseData(int16_t accel[3], int16_t gyro[3], int16_t& temp);

	private:
		I2C& bus;
		const uint8_t addr;
		Config config;

		static constexpr uint16_t transferSize = 32;
		__attribute__((aligned(32))) uint8_t txBuffer[transferSize];
		__attribute__((aligned(32))) uint8_t rxBuffer[transferSize];

		static constexpr float accelSens[] = {
			2.0f / 32768.0f,
			4.0f / 32768.0f,
			8.0f / 32768.0f,
			16.0f / 32768.0f		
		};

		static constexpr float gyroSens[] = {
			250.0f / 32768.0f,
			500.0f / 32768.0f,
			1000.0f / 32768.0f,
			2000.0f / 32768.0f			
		};

		static constexpr float tempSens = (1.0f/340);
		
		float accelOffset[3];
		float gyroOffset[3];
		static constexpr float tempOffset = 25.0f;

		Status WriteRegister(Register reg, uint8_t value);
		Status ReadRegister(Register reg, uint8_t& value);
		Status ModifyRegister(Register reg, uint8_t mask, uint8_t value);
};