#pragma once

// Base MCU peripheral drivers
#include "gpio.hpp"
#include "i2c.hpp"
#include "spi.hpp"
#include "uart.hpp"

// Misc sensor/driver drivers
#include "ina700.hpp"
#include "mpu6050.hpp"
#include "tmc6460.hpp"

extern GPIO ledRed;
extern GPIO ledBlue;
extern GPIO ledGreen;
extern GPIO userButton;

extern GPIO csX;
extern GPIO faultX;
extern GPIO drvEnX;
extern GPIO csY;
extern GPIO faultY;
extern GPIO drvEnY;
extern GPIO csZ;
extern GPIO faultZ;
extern GPIO drvEnZ;

extern I2C i2c1;
extern I2C i2c2;

extern SPI spi1;

extern UART uart1;

extern INA700 ina700;
extern MPU6050 mpu6050;

extern TMC6460 tmc6460X;
extern TMC6460 tmc6460Y;
extern TMC6460 tmc6460Z;

void HardwareInit();