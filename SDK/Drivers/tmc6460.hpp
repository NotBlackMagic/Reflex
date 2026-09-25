/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright (c) 2026 NotBlackMagic (PlumaLabs)
 *
 * File:    SDK/Drivers/tmc6460.hpp
 * Author:  NotBlackMagic
 * Brief:   TMC6460 BLDC FOC driver class for STM32N6.
 */

 #pragma once

#include <stdint.h>

#include "gpio.hpp"
#include "spi.hpp"
#include "status.hpp"
#include "system.hpp"

class TMC6460 {
	public:
		// Standard Chip identifications
		static constexpr uint32_t chipID = 0x36343630;
		static constexpr uint32_t chipVariant = 0x00000000;
		static constexpr uint32_t chipRevision = 0x00000013;

		/// @brief Device register map.
		enum class Register : uint16_t {
			CHIP_ID = 0x00,
			CHIP_VARIANT = 0x01,
			CHIP_REVISION = 0x02,
			CHIP_INPUTS_RAW = 0x04,
			CHIP_OUTPUTS_RAW = 0x05,
			CHIP_IO_MATRIX = 0x06,
			CHIP_IO_PU_PD = 0x07,
			CHIP_IO_CONFIG = 0x08,
			CHIP_STATUS_FLAGS = 0x09,
			CHIP_EVENT = 0x0A,
			CHIP_FAULTn_INT_MASK = 0x0B,
			CHIP_SPI_STATUS_MASK = 0x0C,
			CLK_CTRL_CONFIG = 0x40,
			CLK_CTRL_STATUS = 0x41,
			ADC_CONFIG = 0x80,
			ADC_VERSION = 0x81,
			ADC_I2_I1_RAW = 0x82,
			ADC_VM_I3_RAW = 0x83,
			ADC_TEMP_RAW = 0x84,
			ADC_AIN_V_AIN_U_RAW = 0x85,
			ADC_AIN_W_RAW = 0x86,
			ADC_STATUS = 0x8A,
			ADC_I123 = 0x8C,
			MCC_ADC_I_GEN_CONFIG = 0xC0,
			MCC_ADC_IW_IU = 0xC1,
			MCC_ADC_IV = 0xC2,
			MCC_ADC_CSA_GAIN = 0xC3,
			MCC_ADC_EVENTS = 0xC4,
			MCC_ADC_DYN_GAIN_LIMITS_4X_3X = 0xC5,
			MCC_ADC_DYN_GAIN_LIMIT_2X = 0xC6,
			MCC_ADC_TEMP_LIMITS = 0xC7,
			MCC_ADC_CURRENT_OVERLOAD = 0xC8,
			MCC_CONFIG_MOTOR_MOTION = 0x100,
			MCC_CONFIG_GDRV = 0x101,
			MCC_CONFIG_PWM = 0x102,
			MCC_CONFIG_PWM_PERIOD = 0x103,
			MCC_CONFIG_BRAKE_CHOPPER_LIMITS = 0x104,
			MCC_CONFIG_STATUS = 0x105,
			MCC_CONFIG_TORQUE_FF_ACC_CONFIG = 0x106,
			MCC_CONFIG_TORQUE_FF_VISC_FRIC_CONFIG = 0x107,
			MCC_CONFIG_TORQUE_FF_COULOMB_FRIC = 0x108,
			MCC_CONFIG_TORQUE_FEEDFORWARD = 0x109,
			FOC_PIC_CONFIG = 0x140,
			FOC_PID_U_S_MAX = 0x141,
			FOC_PID_FLUX_COEFF = 0x142,
			FOC_PID_TORQUE_COEFF = 0x143,
			FOC_PID_FIELDWEAK_COEFF = 0x144,
			FOC_PID_VELOCITY_COEFF = 0x145,
			FOC_PID_POSITION_COEFF = 0x146,
			FOC_PID_POSITION_TOLERANCE = 0x147,
			FIC_PID_POSITION_TOLERANCE_DELAY = 0x148,
			FOC_PID_UQ_UD_LIMITS = 0x149,
			FOC_PID_TORQUE_FLUX_LIMITS = 0x14A,
			FOC_PID_VELOCITY_LIMIT = 0x14B,
			FOC_PID_POSITION_LIMIT_LOW = 0x14C,
			FOC_PID_POSITION_LIMIT_HIGH = 0x14D,
			FOC_PID_TORQUE_FLUX_TARGET = 0x14E,
			FOC_PID_TORQUE_FLUX_OFFSET = 0x14F,
			FOC_PID_VELOCITY_TARGET = 0x150,
			FOC_PID_VELOCITY_OFFSET = 0x151,
			FOC_PID_POSITION_TARGET = 0x152,
			FOC_PID_TORQUE_FLUX_ACTUAL = 0x153,
			FOC_PID_VELOCITY_ACTUAL = 0x154,
			FOC_PID_POSITION_ACTUAL = 0x155,
			FOC_PID_POSITION_ACTUAL_OFFSET = 0x156,
			FOC_PID_TORQUE_ERROR = 0x157,
			FOC_PID_FLUX_ERROR = 0x158,
			FOC_PID_VELOCITY_ERROR = 0x159,
			FOC_PID_VELOCITY_ERROR_MAX = 0x15A,
			FOC_PID_POSITION_ERROR = 0x15B,
			FOC_PID_POSITION_ERROR_MAX = 0x15C,
			FOC_PID_TORQUE_INTEGRATOR = 0x15D,
			FOC_PID_FLUX_INTEGRATOR = 0x15E,
			FOC_PID_VELOCITY_INTEGRATOR = 0x15F,
			FOC_PID_POSITION_INTEGRATOR = 0x160,
			FOC_PIDIN_TORQUE_FLUX_TARGET = 0x161,
			FOC_PIDIN_VELOCITY_TARGET = 0x162,
			FOC_PIDIN_POSITION_TARGET = 0x163,
			FOC_PIDIN_TORQUE_FLUX_TARGET_LIMITED = 0x164,
			FOC_PIDIN_VELOCITY_TARGET_LIMITED = 0x165,
			FOC_PIDIN_POSITION_TARGET_LIMITED = 0x166,
			FOC_IBETA_IALPHA = 0x167,
			FOC_IQ_ID = 0x168,
			FOC_UQ_ID = 0x169,
			FOC_UQ_UD_LIMITED = 0x16A,
			FOC_UBETA_UALPHA = 0x16B,
			FOC_UW_UU = 0x16C,
			FOC_UV = 0x16D,
			FOC_PWM_V_U = 0x16E,
			FOC_PWM_W = 0x16F,
			FOC_STATUS = 0x170,
			FOC_U_S_ACTUAL_I_S_ACTUAL = 0x171,
			FOC_P_MOTOR = 0x172,
			FOC_I_T_ACTUAL = 0x173,
			FOC_PRBS_AMPLITUDE = 0x174,
			FOC_PRBS_DOWN_SAMPLING_RATIO = 0x175,
			BIQUAD_EN = 0x180,
			BIQUAD_VELOCITY_A1 = 0x181,
			BIQUAD_VELOCITY_A2 = 0x182,
			BIQUAD_VELOCITY_B0 = 0x183,
			BIQUAD_VELOCITY_B1 = 0x184,
			BIQUAD_VELOCITY_B2 = 0x185,
			BIQUAD_TORQUE_A1 = 0x186,
			BIQUAD_TORQUE_A2 = 0x187,
			BIQUAD_TORQUE_B0 = 0x188,
			BIQUAD_TORQUE_B1 = 0x189,
			BIQUAD_TORQUE_B2 = 0x18A,
			RAMPER_TIME_CONFIG = 0x1C0,
			RAMPER_SWITCH_MODE = 0x1C1,
			RAMPER_PHI_E = 0x1C3,
			RAMPER_A1 = 0x1C4,
			RAMPER_A2 = 0x1C5,
			RAMPER_A_MAX = 0x1C6,
			RAMPER_D1 = 0x1C7,
			RAMPER_D2 = 0x1C8,
			RAMPER_D_MAX = 0x1C9,
			RAMPER_V_START = 0x1CA,
			RAMPER_V1 = 0x1CB,
			RAMPER_V2 = 0x1CC,
			RAMPER_V_STOP = 0x1CD,
			RAMPER_V_MAX = 0x1CE,
			RAMPER_ACCELERATION = 0x1CF,
			RAMPER_V_ACTUAL = 0x1D0,
			RAMPER_POSITION = 0x1D1,
			RAMPER_POSITION_LATCH = 0x1D2,
			RAMPER_POSITION_ACTUAL_LATCH = 0x1D3,
			RAMPER_STATUS = 0x1D4,
			RAMPER_EVENTS = 0x1D5,
			EXT_CTRL_VOLTAGE = 0x200,
			EXT_CTRL_PWM_V_U = 0x202,
			EXT_CTRL_PWM_W = 0x203,
			FEEDBACK_CONF_CH_A = 0x240,
			FEEDBACK_CONF_CH_B = 0x241,
			FEEDBACK_PHI_E_OFFSET = 0x242,
			FEEDBACK_LUT = 0x243,
			FEEDBACK_VELOCITY_FRQ_CONF = 0x244,
			FEEDBACK_VELOCITY_PER_CONF = 0x245,
			FEEDBACK_VELOCITY_PER_FILTER = 0x246,
			FEEDBACK_PHI_CONVERTED = 0x247,
			FEEDBACK_CH_A = 0x248,
			FEEDBACK_CH_B = 0x249,
			FEEDBACK_VELOCITY_FRQ = 0x24A,
			FEEDBACK_VELOCITY_PER = 0x24B,
			FEEDBACK_LUT_WDATA = 0x24C,
			FEEDBACK_PHI_EXT_A = 0x24D,
			FEEDBACK_PHI_EXT_B = 0x24E,
			FEEDBACK_VELOCITY_EXT = 0x24F,
			FEEDBACK_OUTPUT_CONF = 0x250,
			FEEDBACK_PHI_E = 0x251,
			FEEDBACK_PHI_DIFF_LIMIT = 0x252,
			ABN_CONFIG = 0x280,
			ABN_COUNT = 0x281,
			ABN_COUNT_N_CAPTURE = 0x282,
			ABN_COUNT_N_WRITE = 0x283,
			ABN_EVENTS = 0x284,
			ABN2_CONFIG = 0x2C0,
			ABN2_COUNT = 0x2C1,
			ABN2_COUNT_N_CAPTURE = 0x2C2,
			ABN2_COUNT_N_WRITE = 0x2C3,
			ABN2_EVENTS = 0x2C4,
			HALL_MAP_CONFIG = 0x300,
			HALL_DIG_COUNT = 0x301,
			HALL_ANA_CONFIG = 0x302,
			HALL_ANA_UX_CONFIG = 0x303,
			HALL_ANA_VN_CONFIG = 0x304,
			HALL_ANA_WY_CONFIG = 0x305,
			HALL_ANA_UX_OUT = 0x306,
			HALL_ANA_VN_OUT = 0x307,
			HALL_ANA_WY_OUT = 0x308,
			HALL_ANA_OUT = 0x309,
			HALL_DIG_EVENTS = 0x30A,
			UART_CONTROL = 0x340,
			UART_TIMEOUT = 0x341,
			UART_STATUS = 0x342,
			UART_EVENTS = 0x343,
			UART_RTMI_CH_0 = 0x344,
			UART_RTMI_CH_1 = 0x345,
			UART_RTMI_CH_2 = 0x346,
			UART_RTMI_CH_3 = 0x347,
			UART_RTMI_CH_4 = 0x348,
			UART_RTMI_CH_5 = 0x349,
			UART_RTMI_CH_6 = 0x34A,
			UART_RTMI_CH_7 = 0x34B,
			IO_CONTROLLER_CONTROL = 0x380,
			IO_CONTROLLER_COMMAND = 0x381,
			IO_CONTROLLER_RESPONSE_0 = 0x382,
			IO_CONTROLLER_RESPONSE_1 = 0x383,
			IO_CONTROLLER_RESPONSE_2 = 0x384,
			IO_CONTROLLER_RESPONSE_3 = 0x385
		};

		/// @brief TMC6460 driver configuration structure.
		struct Config {
			// Hardware IO
			GPIO* cs;
			GPIO* drvEn;
			// Configurations
			uint8_t motorPoles;
		};

		/// @brief Constructor.
		/// @param spi	Reference to the low-level bus driver.
		TMC6460(SPI& spi) : bus(spi) {};

		/// @brief Initializes the TMC6460 driver.
		/// @param config TMC6460 driver configuration.
		/// @return Status::Ok if initialization succeeded, or Status::Error if the config was invalid or failed.
		Status Init(const Config& config);

		/// @brief Resets the sensor device, using software reset.
		/// @return Status::Ok if reset succeeded cleanly.
		Status Reset();

		/// @brief Reads the Manufacturer and Device IDs.
		/// @param id Device ID, or manufacturer ID.
		/// @return Status::Ok if read succeeded, or Status::Error if failed.
		Status ReadID(uint32_t& id);

		// Standard Control Functions
		Status EnableDriver();
		Status DisableDriver();
		Status ClearFaults();
		Status GetSystemStatus(uint32_t& flags);

		// Motion and FOC targets
		Status SetTorque(int32_t torque, int32_t flux);
		Status SetVelocity(int32_t velocity);
		Status SetPosition(int32_t position);

		// Motion and FOC values
		Status GetTorque(int32_t& torque, int32_t& flux);
		Status GetVelocity(int32_t& velocity);
		Status GetPosition(int32_t& position);
		Status GetFeedback(uint16_t& phiE, uint16_t& phiConvA, uint16_t& phiLookA, uint16_t& phiExtA);
		Status GetCurrent(int32_t& curr1, int32_t& curr2, int32_t& curr3);
		Status GetTemperature(int32_t& internal, int32_t& external);

		// Feedback Engine Configuration 
		/// @brief Configures the TMC to use external SPI angle injection
		Status SetFeedbackScaling(uint32_t scaling);

		/// @brief High-speed function to write the IMU-derived angle
		Status WriteFeedback(uint32_t externalAngle);

		// FOC & Current Tuning
		/// @brief Sets the maximum allowable current (OCP and Soft limits)
		Status SetFOCMode(uint8_t mode);
		Status SetCurrentLimits(uint32_t current);
		Status SetTorqueLimits(int32_t torque, int32_t flux);
		Status SetVelocityLimits(int32_t velocity);
		Status SetPositionLimits(int32_t upper, int32_t lower);

		// Ramp Generator Configuration
        Status SetRampLimits(uint32_t aMax, uint32_t vMax, uint32_t dMax = 0);
		Status SetMotionMode(uint8_t rampMode, uint8_t motionMode);

		/// @brief Configures the Torque and Flux PI coefficients
		/// @note Gains in Q8.8 format
		Status SetCurrentPI(uint16_t pGain, uint16_t iGain);
		Status SetVelocityPI(uint16_t pGain, uint16_t iGain);
		Status SetPositionPI(uint16_t pGain, uint16_t iGain);

		// Calibration routines required for FOC
		Status RunEncoderAlignment(uint16_t alignmentVoltage);

	private:
		SPI& bus;
		Config config;

		int32_t magneticOffset{0};

		static constexpr uint16_t transferSize = 32;
		__attribute__((aligned(32))) uint8_t txBuffer[transferSize];
		__attribute__((aligned(32))) uint8_t rxBuffer[transferSize];

		Status WriteRegister(Register reg, uint32_t value);
		Status ReadRegister(Register reg, uint32_t& value);
		Status ModifyRegister(Register reg, uint32_t mask, uint32_t value);
};