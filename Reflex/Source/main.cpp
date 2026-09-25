#include "stm32c0xx.h"
#include "stm32c0xx_ll_tim.h"

#include "console.hpp"
#include "fusion.hpp"
#include "gpio.hpp"
#include "hardware.hpp"
#include "logger.hpp"
#include "shell.hpp"
#include "system.hpp"

#include <stdio.h>

void UpdateFastControl();
void ProcessCommands();
void UpdateTelemetry();

// Global flag to protect the SPI bus from collisions
volatile bool pauseFOCUpdates = false;

// Global variables to hold the latest IMU data
int16_t accel[3] = {0, 0, 0};
int16_t gyro[3] = {0, 0, 0};
int16_t temp = 0;
int16_t angles[3] = {0, 0, 0};

// Gimbal BLDC Motor (GB1105) Characteristics
const uint8_t motorPoles = 6;			// Number of pole pairs
const uint16_t phaseResistance = 12500;	// Phase resistance, in mOhm, 12.5Ohm
const uint8_t motorTurns = 50;			// Winding turns of motor
const uint16_t stallCurrent = 920;		// Stall current, in mA
const uint8_t stallTorque = 10;			// Stall torque, in mN*m
const uint16_t nomCurrent = 590;		// Nominal current, in mA
const uint16_t nomVoltage = 12;			// Nominal voltage, in V
const uint16_t nomTorque = 10;			// Nominal torque, in mN*m

int main(void) {
	System::InitClock();
	System::InitSysTick();

	// Init Logger and Console
	Logger::Instance().Init();
	Logger::Instance().RegisterConsole(&uart1);

	LOG_INFO("--------------------------------");
	LOG_INFO("System Booting...");
	LOG_INFO("Logger Initialized.");

	// Start Peripherals
	HardwareInit();

	// Initialize Timers
	// TIM3
	// Enable bus clocks
	LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_TIM3);

	// Configure the Timer: TIM3 is connected to APB1 Timer clocks which has the APB1 clock = 48MHz
	LL_TIM_SetPrescaler(TIM3, 47);					// Set Clock to 1kHz (1ms); Fpwm = 48MHz / ((Prescaler + 1) * Compare)
	LL_TIM_SetCounterMode(TIM3, LL_TIM_COUNTERMODE_UP);
	LL_TIM_SetAutoReload(TIM3, 1000);
	LL_TIM_EnableARRPreload(TIM3);

	// This is needed to update the prescaler
	LL_TIM_GenerateEvent_UPDATE(TIM3);

	// Configure TIM1 Interrupts
	NVIC_SetPriority(TIM3_IRQn, 5);
	NVIC_EnableIRQ(TIM3_IRQn);

	// Start Console
	Console::Init(&uart1);

	LOG_INFO("Peripherals Initialized.");

	// Init sensors and drivers
	volatile uint8_t probeI2C1 = i2c1.Probe(0x44);

	// Initialize
	if(i2c1.Probe(0x44) == 0x01) {
		INA700::Config config = {};
		if(ina700.Init(config) == Status::Ok) {
			LOG_INFO("PW Monitor (INA700) Init OK");
		}
		else {
			LOG_WARN("PW Monitor (INA700) Init Failed!");
		}
	}
	else {
		LOG_WARN("PW Monitor (INA700) not found on I2C1!");
	}

	if(i2c1.Probe(0x68) == 0x01) {
		MPU6050::Config config = {
			.accelScale = MPU6050::AccelScale::G16,
			.gyroScale = MPU6050::GyroScale::DPS1000,
			.accelOdr = MPU6050::OutputDataRate::Hz1600,
			.gyroOdr = MPU6050::OutputDataRate::Hz1600
		};
		if(mpu6050.Init(config) == Status::Ok) {
			LOG_INFO("Gimbal IMU (MPU6050) Init OK");
		}
		else {
			LOG_WARN("Gimbal IMU (MPU6050) Init Failed!");
		}
	}
	else {
		LOG_WARN("Gimbal IMU (MPU6050) not found on I2C1!");
	}

	if(tmc6460X.Init({.cs = &csX, .drvEn = &drvEnX, .motorPoles = motorPoles}) == Status::Ok) {
		// Set the scaling so 36000 counts = 1 full revolution
		tmc6460X.SetFeedbackScaling(466);

		// Clear any startup faults before entering the main loop
		tmc6460X.ClearFaults();

		LOG_INFO("BLDC Driver X (TMC6460) Init OK");
	}
	else {
		LOG_INFO("BLDC Driver X (TMC6460) Init Failed!");
	}

	if(tmc6460Y.Init({.cs = &csY, .drvEn = &drvEnY, .motorPoles = motorPoles}) == Status::Ok) {
		// Set the scaling so 36000 counts = 1 full revolution
		tmc6460Y.SetFeedbackScaling(466);

		// Clear any startup faults before entering the main loop
		tmc6460Y.ClearFaults();
		
		LOG_INFO("BLDC Driver Y (TMC6460) Init OK");
	}
	else {
		LOG_INFO("BLDC Driver Y (TMC6460) Init Failed!");
	}

	if(tmc6460Z.Init({.cs = &csZ, .drvEn = &drvEnZ, .motorPoles = motorPoles}) == Status::Ok) {
		// Set the scaling so 36000 counts = 1 full revolution
		tmc6460Z.SetFeedbackScaling(466);

		// Clear any startup faults before entering the main loop
		tmc6460Z.ClearFaults();
		
		LOG_INFO("BLDC Driver Z (TMC6460) Init OK");
	}
	else {
		LOG_INFO("BLDC Driver Z (TMC6460) Init Failed!");
	}

	// Enable Timer, with clean state
	LL_TIM_EnableCounter(TIM3);
	LL_TIM_ClearFlag_UPDATE(TIM3);
	LL_TIM_EnableIT_UPDATE(TIM3);

	uint32_t commandUpdate = Time::GetMs();
	uint32_t telemetryUpdate = Time::GetMs();
	while(1) {
		uint32_t timestamp = Time::GetMs();

		// 100Hz loop
		if((timestamp - commandUpdate) >= 10) {
			ProcessCommands();
			commandUpdate = timestamp;
		}

		// 10Hz loop
		if((timestamp - telemetryUpdate) >= 100) {
			UpdateTelemetry();
			telemetryUpdate = timestamp;
		}
	}
}

void UpdateFastControl() {
	// Start the non-blocking I2C transfer
	if(mpu6050.RequestData() != Status::Ok) {
		i2c1.TransferAbort();
	}
	ledBlue.Toggle();
}

void ProcessCommands() {
	Console::Run();
	ledGreen.Toggle();
}

void UpdateTelemetry() {
	float voltage, current, temperature;
	ina700.ReadVoltage(voltage);
	ina700.ReadCurrent(current);
	ina700.ReadTemperature(temperature);

	// LOG_INFO("PW: %dmV %dmA %dC", (int)(voltage * 1000), (int)(current * 1000), (int)temperature);
	// LOG_INFO("STATE: P: %d, R: %d, Y: %d", (angles[0] / 100), (angles[1] / 100), (angles[2] / 100));
}

/// @brief I2C1 interrupt callback, port connected to IMU (MPU-6050).
/// @note Called when all requested data is ready, convert to angles and feed to BLDC driver (TMC6460)
/// @param ctx		Pointer to context
/// @param event	I2C event that caused the callback
/// @return None
void I2C1EventHandler(void* ctx, I2C::Event event) {
	(void)ctx; // Unused

	if(event == I2C::Event::TransferComplete) {
		// Parse received data buffer
		mpu6050.ParseData(accel, gyro, temp);

		// Sensor fusion, get gimbal rotation and mapping to BLDC motor axis
		SensorFusion::Update(accel, gyro);

		angles[0] = SensorFusion::GetPitch();
		angles[1] = SensorFusion::GetRoll();
		angles[2] = SensorFusion::GetYaw();

		// Limit to 0 - 360 (unsigned angle)
		if(angles[0] < 0) {
			angles[0] = 36000 + angles[0];
		}
		if(angles[1] < 0) {
			angles[1] = 36000 + angles[1];
		}
		if(angles[2] < 0) {
			angles[2] = 36000 + angles[2];
		}

		// Feedback angles to TMC
		if(pauseFOCUpdates == false) {
			tmc6460X.WriteFeedback(angles[0]);
			tmc6460Y.WriteFeedback(angles[1]);
			tmc6460Z.WriteFeedback(angles[2]);
		}
	}
}

/// @brief TIM3 interrupt handler, used for state update for control loop. Trigger a read from the IMU (MPU-6050)
/// @note Triggers/starts a read from the IMU (MPU-6050), non-blocking, read is finalized in I2C callback (I2C1EventHandler)
/// @return None
extern "C" void TIM3_IRQHandler(void) {
	if(LL_TIM_IsActiveFlag_UPDATE(TIM3) == 0x01) {
		UpdateFastControl();
		LL_TIM_ClearFlag_UPDATE(TIM3);
	}
}