/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright (c) 2026 NotBlackMagic (PlumaLabs)
 *
 * File:    SDK/Drivers/Sensors/mpu6050.cpp
 */

#include "mpu6050.hpp"

Status MPU6050::Init(const Config& config) {
	this->config = config;

	// Verify ID
	uint8_t id;
	this->ReadID(id);
	if(id != this->chipID) {
		return Status::Error;
	}

	// Reset device
	Status status = this->Reset();
	if(status != Status::Ok) {
		return status;
	}

	// Turn on device in ??? Mode
	status = this->WriteRegister(MPU6050::Register::PWR_MGMT_1, 0x00);
	if(status != Status::Ok) {
		return status;
	}

	// Configure scales and rates
	status = this->ModifyRegister(MPU6050::Register::ACCEL_CONFIG, 0x18, (static_cast<uint8_t>(this->config.accelScale) << 3));
	if(status != Status::Ok) {
		return status;
	}
	status = this->ModifyRegister(MPU6050::Register::GYRO_CONFIG, 0x18, (static_cast<uint8_t>(this->config.gyroScale) << 3));
	if(status != Status::Ok) {
		return status;
	}

	// Configure INT1 outputs
	status = this->ModifyRegister(MPU6050::Register::INT_PIN_CFG, 0x30, 0x30);
	if(status != Status::Ok) {
		return status;
	}
	status = this->ModifyRegister(MPU6050::Register::INT_ENABLE, 0x01, 0x01);
	if(status != Status::Ok) {
		return status;
	}

	return Status::Ok;
}

Status MPU6050::Reset() {
	return Status::Ok;
}

Status MPU6050::ReadID(uint8_t& id) {
	return this->ReadRegister(MPU6050::Register::WHO_AM_I, id);
}

Status MPU6050::SetScales(AccelScale accelScale, GyroScale gyroScale) {
	Status status = this->ModifyRegister(MPU6050::Register::ACCEL_CONFIG, 0x18, (static_cast<uint8_t>(accelScale) << 3));
	if(status != Status::Ok) {
		return status;
	}
	this->config.accelScale = accelScale;

	status = this->ModifyRegister(MPU6050::Register::GYRO_CONFIG, 0x18, (static_cast<uint8_t>(gyroScale) << 3));
	if(status != Status::Ok) {
		return status;
	}
	this->config.gyroScale = gyroScale;

	return Status::Ok;
}

Status MPU6050::SetAccelOffsets(float offsetX, float offsetY, float offsetZ) {
	this->accelOffset[0] = offsetX;
	this->accelOffset[1] = offsetY;
	this->accelOffset[2] = offsetZ;
	return Status::Ok;
}

Status MPU6050::SetGyroOffsets(float offsetX, float offsetY, float offsetZ) {
	this->gyroOffset[0] = offsetX;
	this->gyroOffset[1] = offsetY;
	this->gyroOffset[2] = offsetZ;
	return Status::Ok;
}

Status MPU6050::RunHardwareSelfTest() {
	return Status::Ok;
}

Status MPU6050::RequestData() {
	this->txBuffer[0] = static_cast<uint8_t>(MPU6050::Register::ACCEL_XOUT_H);
	this->txBuffer[1] = 0x00; // Dummy byte
	Status status = this->bus.TransferAsync(this->addr, this->txBuffer, 1, this->rxBuffer, 14);
	return status;
}

void MPU6050::ParseData(int16_t accel[3], int16_t gyro[3], int16_t& temp) {
	accel[0] = static_cast<int16_t>((this->rxBuffer[0] << 8) | this->rxBuffer[1]);
	accel[1] = static_cast<int16_t>((this->rxBuffer[2] << 8) | this->rxBuffer[3]);
	accel[2] = static_cast<int16_t>((this->rxBuffer[4] << 8) | this->rxBuffer[5]);
	temp = static_cast<int16_t>((this->rxBuffer[6] << 8) | this->rxBuffer[7]);
	gyro[0] = static_cast<int16_t>((this->rxBuffer[8] << 8) | this->rxBuffer[9]);
	gyro[1] = static_cast<int16_t>((this->rxBuffer[10] << 8) | this->rxBuffer[11]);
	gyro[2] = static_cast<int16_t>((this->rxBuffer[12] << 8) | this->rxBuffer[13]);
}

Status MPU6050::WriteRegister(Register reg, uint8_t value) {
	this->txBuffer[0] = static_cast<uint8_t>(reg);
	this->txBuffer[1] = value;
	Status status = this->bus.TransferAsync(this->addr, this->txBuffer, 2, nullptr, 2);
	if(status != Status::Ok) {
		return status;
	}

	return this->bus.TransferWait(1000);
}

Status MPU6050::ReadRegister(Register reg, uint8_t& value) {
	this->txBuffer[0] = static_cast<uint8_t>(reg);
	this->txBuffer[1] = 0x00; // Dummy byte
	Status status = this->bus.TransferAsync(this->addr, this->txBuffer, 1, this->rxBuffer, 1);
	if(status != Status::Ok) {
		return status;
	}

	status = this->bus.TransferWait(1000);
	value = this->rxBuffer[0];

	return status;
}

Status MPU6050::ModifyRegister(Register reg, uint8_t mask, uint8_t value) {
	uint8_t tmp;

	Status status = this->ReadRegister(reg, tmp);
	if(status != Status::Ok) {
		return status;
	}

	tmp &= ~mask;
	tmp |= value;

	return this->WriteRegister(reg, tmp);
}