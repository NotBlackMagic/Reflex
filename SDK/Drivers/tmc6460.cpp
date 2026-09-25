/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright (c) 2026 NotBlackMagic (PlumaLabs)
 *
 * File:	SDK/Drivers/tmc6460.cpp
 */

#include "tmc6460.hpp"

Status TMC6460::Init(const Config& config) {
	this->config = config;

	// Verify ID
	uint32_t id;
	this->ReadID(id);
	if(id != this->chipID) {
		return Status::Error;
	}

	// Reset device
	Status status = this->Reset();
	if(status != Status::Ok) {
		return status;
	}

	// Configure device
	// General configurations
	// status = this->WriteRegister(TMC6460::Register::MCC_CONFIG_MOTOR_MOTION, 0x00000E780 + (this->config.motorPoles));	// BLDC motor and velocity control used, open-loop (feedback through PHI_E) and RAMP_EN and RAMP_MODE VEL
	uint8_t rampEn = 0;		// 0: Ramp mode disabled, 1: Ramp mode enabled
	uint8_t rampMode = 0;	// 0: RAMP_POSITION or 1: RAMP_VELOCITY
	uint8_t motionMode = 4;	// 0: PWM_OFF, 1: PWM_ON, 2: TORQUE, 3: VELOCITY, 4: POSITION, 5: PRBS_UD, 6: PRBS_FLUX, 7: PRBS_TORQUE, 8: PRBS_VELOCITY, 9: PRBS_POSITION, 10: PWM_EXT, 11: VOLTAGE_EXT
	uint8_t motorType = 3;	// 0: None, 1: DC, 3: BLDC
	status = this->WriteRegister(TMC6460::Register::MCC_CONFIG_MOTOR_MOTION, (static_cast<uint32_t>(rampEn) << 14) |
																						(static_cast<uint32_t>(rampMode) << 13) | 
																						(static_cast<uint32_t>(motionMode) << 9) |
																						(static_cast<uint32_t>(motorType) << 7) |
																						(this->config.motorPoles));

	if(status != Status::Ok) {
		return status;
	}
	status = this->WriteRegister(TMC6460::Register::MCC_CONFIG_GDRV, 0x81003401);		// Set LR_RES_ON to 0b00 i.e. current full scale 1.25A
	if(status != Status::Ok) {
		return status;
	}

	// Feedback configurations
	uint16_t srcSel = 7;	// PHI_EXT_A
	uint32_t scaling = 466;	// Angles from IMU/Fusion are [0, 36000] degrees, scaling to convert to 2^16 and is in Q16.8
	status = this->WriteRegister(Register::FEEDBACK_CONF_CH_A, (static_cast<uint32_t>(srcSel) << 24) | (scaling & 0x00FFFFFF));
	if(status != Status::Ok) {
		return status;
	}
	status = this->WriteRegister(Register::FEEDBACK_OUTPUT_CONF, (this->config.motorPoles));	// Set PHI_E_MUL_FACT to number of poles
	if(status != Status::Ok) {
		return status;
	}

	// PI-Loops configurations
	uint16_t fluxLim = 5000;
	uint16_t torqueP = 1000;
	uint16_t torqueI = 100;
	uint16_t torqueLim = 5000;
	uint16_t fieldWeakP = 1000;
	uint16_t fieldWeakI = 0;

	uint16_t velP = 1000;
	uint16_t velI = 100;
	uint32_t velLim = 0x7FFFFFFF;

	uint16_t posP = 1000;
	uint16_t posI = 100;
	uint32_t posLimLow = 0x80000001;
	uint32_t posLimUp = 0x7FFFFFFF;
	status = this->WriteRegister(Register::FOC_PID_TORQUE_COEFF, (static_cast<uint32_t>(torqueP) << 16) | torqueI);
	if(status != Status::Ok) {
		return status;
	}
	status = this->WriteRegister(Register::FOC_PID_TORQUE_FLUX_LIMITS, (static_cast<uint32_t>(torqueLim) << 16) | fluxLim);
	if(status != Status::Ok) {
		return status;
	}
	status = this->WriteRegister(Register::FOC_PID_FIELDWEAK_COEFF, (static_cast<uint32_t>(fieldWeakP) << 16) | fieldWeakI);
	if(status != Status::Ok) {
		return status;
	}
	status = this->WriteRegister(Register::FOC_PID_VELOCITY_COEFF, (static_cast<uint32_t>(velP) << 16) | velI);
	if(status != Status::Ok) {
		return status;
	}
	status = this->WriteRegister(Register::FOC_PID_VELOCITY_LIMIT, velLim);
	if(status != Status::Ok) {
		return status;
	}
	status = this->WriteRegister(Register::FOC_PID_POSITION_COEFF, (static_cast<uint32_t>(posP) << 16) | posI);
	if(status != Status::Ok) {
		return status;
	}
	status = this->WriteRegister(Register::FOC_PID_POSITION_LIMIT_LOW, posLimLow);
	if(status != Status::Ok) {
		return status;
	}
	status = this->WriteRegister(Register::FOC_PID_POSITION_LIMIT_HIGH, posLimUp);
	if(status != Status::Ok) {
		return status;
	}

	return Status::Ok;
}

Status TMC6460::Reset() {
	return Status::Ok;
}

Status TMC6460::ReadID(uint32_t& id) {
	return this->ReadRegister(Register::CHIP_ID, id);
}

Status TMC6460::EnableDriver() {
	if(this->config.cs != nullptr) {
		this->config.drvEn->Write(1);
	}
	return this->ModifyRegister(Register::MCC_CONFIG_GDRV, 0x00010000, 0x00010000);
}

Status TMC6460::DisableDriver() {
	Status status = this->ModifyRegister(Register::MCC_CONFIG_GDRV, 0x00010000, 0x00000000);
	if(this->config.cs != nullptr) {
		this->config.drvEn->Write(0);
	}
	return status;
}

Status TMC6460::ClearFaults() {
	// Clear all event latches
	Status status = this->WriteRegister(Register::CHIP_EVENT, 0xFFFFFFFF);
	if(status == Status::Ok) {
		status = this->WriteRegister(Register::RAMPER_EVENTS, 0xFFFFFFFF);
	}
	return status;
}

Status TMC6460::SetFeedbackScaling(uint32_t scaling) {
	// Bits [27:24] select the source (0x07 for PHI_EXT_A)
	// Bits [23:0] define CPR_INV_A. The scaling of the feedback value to 16-bit i.e.: converted_angle = (selected_source_angle × CPR_INV_A) >> 8. Normally set to 2^24/CPR (CPR=Counts-per-revolution)
	uint16_t srcSel = 7;	//PHI_EXT_A
	return this->WriteRegister(Register::FEEDBACK_CONF_CH_A, (static_cast<uint32_t>(srcSel) << 24) | (scaling & 0x00FFFFFF));
}

Status TMC6460::WriteFeedback(uint32_t value) {
	return this->WriteRegister(Register::FEEDBACK_PHI_EXT_A, value);
}

Status TMC6460::GetSystemStatus(uint32_t& flags) {
	return this->ReadRegister(Register::CHIP_STATUS_FLAGS, flags);
}

Status TMC6460::SetCurrentLimits(uint32_t current) {
	return this->WriteRegister(Register::MCC_ADC_CURRENT_OVERLOAD, current);
}

Status TMC6460::SetTorqueLimits(int32_t torque, int32_t flux) {
	return this->WriteRegister(Register::FOC_PID_TORQUE_FLUX_LIMITS, (static_cast<uint32_t>(torque) << 16) | flux);
}

Status TMC6460::SetVelocityLimits(int32_t velocity) {
	return this->WriteRegister(Register::FOC_PID_VELOCITY_LIMIT, velocity);
}

Status TMC6460::SetPositionLimits(int32_t upper, int32_t lower) {
	Status status = this->WriteRegister(Register::FOC_PID_POSITION_LIMIT_LOW, lower);
	if(status != Status::Ok) {
		return status;
	}
	return this->WriteRegister(Register::FOC_PID_POSITION_LIMIT_HIGH, upper);
}

Status TMC6460::SetRampLimits(uint32_t aMax, uint32_t vMax, uint32_t dMax) {
	// If no explicit deceleration is provided, make the ramp symmetric
	uint32_t decel = (dMax == 0) ? aMax : dMax;

	Status status = this->WriteRegister(Register::RAMPER_A_MAX, aMax);
	if(status == Status::Ok) {
		status = this->WriteRegister(Register::RAMPER_D_MAX, decel);
	}
	if(status == Status::Ok) {
		status = this->WriteRegister(Register::RAMPER_V_MAX, vMax);
	}

	// Zero out the intermediate velocity thresholds to bypass the complex 8-point curve and force a standard 3-phase trapezoidal ramp.
	if(status == Status::Ok) {
		status = this->WriteRegister(Register::RAMPER_V1, 0);
	}
	if(status == Status::Ok) {
		status = this->WriteRegister(Register::RAMPER_V2, 0);
	}

	return status;
}

Status TMC6460::SetTorque(int32_t torque, int32_t flux) {
	return this->WriteRegister(Register::FOC_PID_TORQUE_FLUX_TARGET, (static_cast<uint32_t>(torque) << 16) | flux);
}

Status TMC6460::SetVelocity(int32_t velocity) {
	return this->WriteRegister(Register::FOC_PID_VELOCITY_TARGET, static_cast<uint32_t>(velocity));
}

Status TMC6460::SetPosition(int32_t position) {
	return this->WriteRegister(Register::FOC_PID_POSITION_TARGET, static_cast<uint32_t>(position));
}

Status TMC6460::GetCurrent(int32_t& curr1, int32_t& curr2, int32_t& curr3) {
	int32_t scale = 13193;	// Q16.16 for (in mA): ICSx [A] = Ix × 0.00164 [A] / 8 × 4/4 × 0.982
	uint32_t tmp = 0;
	Status status = this->ReadRegister(Register::ADC_I2_I1_RAW, tmp);
	if(status != Status::Ok) {
		return status;
	}
	curr1 = (static_cast<int16_t>(tmp & 0xFFFF) * scale) >> 16;
	curr2 = (static_cast<int16_t>(tmp >> 16) * scale) >> 16;
	status = this->ReadRegister(Register::ADC_VM_I3_RAW, tmp);
	curr3 = (static_cast<int16_t>(tmp & 0xFFFF) * scale) >> 16;
	return status;
}

Status TMC6460::GetTemperature(int32_t& internal, int32_t& external) {
	int32_t scaleExt = 11250;		// Q16.16 for (in mV):
	uint32_t tmp = 0;
	Status status = this->ReadRegister(Register::ADC_TEMP_RAW, tmp);
	external = (static_cast<int16_t>(tmp & 0xFFFF) * scaleExt) >> 16;
	internal = ((static_cast<int16_t>(tmp >> 16) * 5 - 81527) * 41667) >> (15 + 2);
	return status;
}

Status TMC6460::GetTorque(int32_t& torque, int32_t& flux) {
	uint32_t tmp;
	Status status = this->ReadRegister(Register::FOC_PID_TORQUE_FLUX_ACTUAL, tmp);
	torque = static_cast<int16_t>(tmp >> 16);
	flux = static_cast<int16_t>(tmp & 0xFFFF);
	return status;
}

Status TMC6460::GetVelocity(int32_t& velocity) {
	uint32_t tmp;
	Status status = this->ReadRegister(Register::FOC_PID_VELOCITY_ACTUAL, tmp);
	velocity = static_cast<int32_t>(tmp);
	return status;
}

Status TMC6460::GetPosition(int32_t& position) {
	uint32_t tmp;
	Status status = this->ReadRegister(Register::FOC_PID_POSITION_ACTUAL, tmp);
	position = static_cast<int32_t>(tmp);
	return status;
}

Status TMC6460::GetFeedback(uint16_t& phiE, uint16_t& phiConvA, uint16_t& phiLookA, uint16_t& phiExtA) {
	uint32_t tmp;
	Status status = this->ReadRegister(Register::FEEDBACK_PHI_E, tmp);
	if(status != Status::Ok) {
		return status;
	}
	phiE = static_cast<uint16_t>(tmp >> 16);
	status = this->ReadRegister(Register::FEEDBACK_PHI_CONVERTED, tmp);
	if(status != Status::Ok) {
		return status;
	}
	phiConvA = static_cast<uint16_t>(tmp & 0xFFFF);
	status = this->ReadRegister(Register::FEEDBACK_CH_A, tmp);
	if(status != Status::Ok) {
		return status;
	}
	phiLookA = static_cast<uint16_t>(tmp & 0xFFFF);
	status = this->ReadRegister(Register::FEEDBACK_PHI_EXT_A, tmp);
	phiExtA = tmp;
	return status;
}

Status TMC6460::SetMotionMode(uint8_t rampMode, uint8_t motionMode) {
	// MOTION_MODE is defined in bits [12:9] of MCC_CONFIG_MOTOR_MOTION
	return this->ModifyRegister(Register::MCC_CONFIG_MOTOR_MOTION, 0x00003E00, (static_cast<uint32_t>(rampMode) << 13) | (static_cast<uint32_t>(motionMode) << 9));
}

Status TMC6460::SetCurrentPI(uint16_t pGain, uint16_t iGain) {
	Status status = this->WriteRegister(Register::FOC_PID_TORQUE_COEFF, (static_cast<uint32_t>(pGain) << 16) | iGain);
	if(status == Status::Ok) {
		status = this->WriteRegister(Register::FOC_PID_FLUX_COEFF, (static_cast<uint32_t>(pGain) << 16) | iGain);
	}
	return status;
}

Status TMC6460::SetVelocityPI(uint16_t pGain, uint16_t iGain) {
	return this->WriteRegister(Register::FOC_PID_VELOCITY_COEFF, (static_cast<uint32_t>(pGain) << 16) | iGain);
}

Status TMC6460::SetPositionPI(uint16_t pGain, uint16_t iGain) {
	return this->WriteRegister(Register::FOC_PID_POSITION_COEFF, (static_cast<uint32_t>(pGain) << 16) | iGain);
}

Status TMC6460::RunEncoderAlignment(uint16_t alignmentVoltage) {
	// Force the internal open-loop angle to 0
	this->WriteRegister(Register::RAMPER_PHI_E, 0);

	// Set UQ to 0 and UD to the alignment voltage
	uint32_t voltageCommand = (static_cast<uint32_t>(alignmentVoltage) & 0xFFFF);
	this->WriteRegister(Register::EXT_CTRL_VOLTAGE, voltageCommand);

	// Switch to External Voltage Mode and enable the driver
	this->SetMotionMode(1, 11);
	this->EnableDriver();

	// Wait for the gimbal axis to snap to 0 and stop swinging.
	Time::Delay(1000);

	// Read the absolute electrical angle from the feedback engine
	uint32_t currentPhiE = 0;
	this->ReadRegister(Register::FEEDBACK_PHI_E, currentPhiE);

	// Because the rotor is physically locked at 0, the currently reported angle IS the exact offset. Write it.
	this->WriteRegister(Register::FEEDBACK_PHI_E_OFFSET, currentPhiE);

	// Disable driver, reset voltage, and return to Position Mode (MOTION_MODE = 4)
	this->DisableDriver();
	this->WriteRegister(Register::EXT_CTRL_VOLTAGE, 0);

	// To velocity control mode
	this->SetMotionMode(1, 3);

	return Status::Ok;
}

Status TMC6460::WriteRegister(Register reg, uint32_t value) {
	if(this->config.cs == nullptr) {
		return Status::Error;
	}

	this->txBuffer[0] = 0x80 | (uint8_t)((static_cast<uint16_t>(reg) >> 8) & 0x03);	//WnR bit and top 2 bits of 10-bit address
	this->txBuffer[1] = (uint8_t)(static_cast<uint16_t>(reg) & 0xFF);				//Lower 8 bits of 10-bit address
	this->txBuffer[2] = (uint8_t)((value >> 24) & 0xFF);
	this->txBuffer[3] = (uint8_t)((value >> 16) & 0xFF);
	this->txBuffer[4] = (uint8_t)((value >> 8) & 0xFF);
	this->txBuffer[5] = (uint8_t)((value) & 0xFF);

	this->config.cs->Write(0);	// Set CS
	Status status = this->bus.TransferAsync(this->txBuffer, this->rxBuffer, 6);
	if(status == Status::Ok) {
		status = this->bus.TransferWait(1000);
	}
	this->config.cs->Write(1);	// Reset CS

	return status;
}

Status TMC6460::ReadRegister(Register reg, uint32_t& value) {
	if(this->config.cs == nullptr) {
		return Status::Error;
	}
	
	// First Transfer: Send the read request
	this->txBuffer[0] = 0x00 + (uint8_t)((static_cast<uint16_t>(reg) >> 8) & 0x03);	//WnR bit and top 2 bits of 10-bit address
	this->txBuffer[1] = (uint8_t)(static_cast<uint16_t>(reg) & 0xFF);				//Lower 8 bits of 10-bit address
	this->txBuffer[2] = 0x00;
	this->txBuffer[3] = 0x00;
	this->txBuffer[4] = 0x00;
	this->txBuffer[5] = 0x00;

	this->config.cs->Write(0);	// Set CS
	Status status = this->bus.TransferAsync(this->txBuffer, this->rxBuffer, 6);
	if(status == Status::Ok) {
		status = this->bus.TransferWait(1000);
	}
	this->config.cs->Write(1);	// Reset CS
	if(status != Status::Ok) {
		return status;
	}

	// Second Transfer: Dummy read (Targeting CHIP_ID) to receive the actual data
	this->txBuffer[0] = 0x00; 
	this->txBuffer[1] = 0x00;

	this->config.cs->Write(0);
	status = this->bus.TransferAsync(this->txBuffer, this->rxBuffer, 6);
	if(status == Status::Ok) {
		status = this->bus.TransferWait(1000);
	}
	this->config.cs->Write(1);
	if(status == Status::Ok) {
		value = (this->rxBuffer[2] << 24);
		value += (this->rxBuffer[3] << 16);
		value += (this->rxBuffer[4] << 8);
		value += this->rxBuffer[5];
	}

	return status;
}

Status TMC6460::ModifyRegister(Register reg, uint32_t mask, uint32_t value) {
	uint32_t tmp;

	Status status = this->ReadRegister(reg, tmp);
	if(status != Status::Ok) {
		return status;
	}

	tmp &= ~mask;
	tmp |= value;

	return this->WriteRegister(reg, tmp);
}