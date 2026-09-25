#include "hardware.hpp"

GPIO ledRed(GPIOB, LL_GPIO_PIN_0);
GPIO ledBlue(GPIOA, LL_GPIO_PIN_8);
GPIO ledGreen(GPIOB, LL_GPIO_PIN_1);
GPIO userButton(GPIOA, LL_GPIO_PIN_15);

GPIO csX(GPIOA, LL_GPIO_PIN_3);
GPIO faultX(GPIOA, LL_GPIO_PIN_4);
GPIO drvEnX(GPIOA, LL_GPIO_PIN_5);
GPIO csY(GPIOB, LL_GPIO_PIN_3);
GPIO faultY(GPIOB, LL_GPIO_PIN_5);
GPIO drvEnY(GPIOB, LL_GPIO_PIN_4);
GPIO csZ(GPIOB, LL_GPIO_PIN_6);
GPIO faultZ(GPIOB, LL_GPIO_PIN_8);
GPIO drvEnZ(GPIOB, LL_GPIO_PIN_7);

// To gimbal IMU and on-board sensor
I2C i2c1(I2C1);
extern "C" void I2C1_IRQHandler(void) { i2c1.InterruptHandler(); }
// To control header (as slave)
I2C i2c2(I2C2);
extern "C" void I2C2_IRQHandler(void) { i2c2.InterruptHandler(); }

// To TMC6460 BLDC FOC Driver (Max. Clock allowed: 8MHz)
SPI spi1(SPI1);
extern "C" void SPI1_IRQHandler(void) { spi1.InterruptHandler(); }

UART uart1(USART1);
extern "C" void USART1_IRQHandler(void) { uart1.InterruptHandler(); }

// On-board sensors and drivers
INA700 ina700(i2c1, 0x44);
MPU6050 mpu6050(i2c1, 0x68);
TMC6460 tmc6460X(spi1);
TMC6460 tmc6460Y(spi1);
TMC6460 tmc6460Z(spi1);

extern void I2C1EventHandler(void* ctx, I2C::Event event);

void HardwareInit() {
	// Initialize Physical Layer

	// Initialize general purpose GPIOs
	LL_IOP_GRP1_EnableClock(LL_IOP_GRP1_PERIPH_GPIOA);
	LL_IOP_GRP1_EnableClock(LL_IOP_GRP1_PERIPH_GPIOB);
	LL_IOP_GRP1_EnableClock(LL_IOP_GRP1_PERIPH_GPIOC);
	LL_IOP_GRP1_EnableClock(LL_IOP_GRP1_PERIPH_GPIOD);
	LL_IOP_GRP1_EnableClock(LL_IOP_GRP1_PERIPH_GPIOF);
	// I2C GPIOs
	// I2C1
	LL_IOP_GRP1_EnableClock(LL_IOP_GRP1_PERIPH_GPIOA);
	LL_IOP_GRP1_EnableClock(LL_IOP_GRP1_PERIPH_GPIOB);
	// Configure GPIOs
	// Set I2C1 SDA on PB9
	LL_GPIO_SetPinMode(GPIOB, LL_GPIO_PIN_9, LL_GPIO_MODE_ALTERNATE);
	LL_GPIO_SetPinSpeed(GPIOB, LL_GPIO_PIN_9, LL_GPIO_SPEED_FREQ_LOW);
	LL_GPIO_SetPinOutputType(GPIOB, LL_GPIO_PIN_9, LL_GPIO_OUTPUT_OPENDRAIN);
	LL_GPIO_SetPinPull(GPIOB, LL_GPIO_PIN_9, LL_GPIO_PULL_UP);
	LL_GPIO_SetAFPin_8_15(GPIOB, LL_GPIO_PIN_9, LL_GPIO_AF_6);
	// Set I2C1 SCL on PA9
	LL_GPIO_SetPinMode(GPIOA, LL_GPIO_PIN_9, LL_GPIO_MODE_ALTERNATE);
	LL_GPIO_SetPinSpeed(GPIOA, LL_GPIO_PIN_9, LL_GPIO_SPEED_FREQ_LOW);
	LL_GPIO_SetPinOutputType(GPIOA, LL_GPIO_PIN_9, LL_GPIO_OUTPUT_OPENDRAIN);
	LL_GPIO_SetPinPull(GPIOA, LL_GPIO_PIN_9, LL_GPIO_PULL_UP);
	LL_GPIO_SetAFPin_8_15(GPIOA, LL_GPIO_PIN_9, LL_GPIO_AF_6);
	// I2C2
	LL_IOP_GRP1_EnableClock(LL_IOP_GRP1_PERIPH_GPIOA);
	LL_IOP_GRP1_EnableClock(LL_IOP_GRP1_PERIPH_GPIOB);
	// Configure GPIOs
	// Set I2C1 SDA on PA10
	LL_GPIO_SetPinMode(GPIOA, LL_GPIO_PIN_10, LL_GPIO_MODE_ALTERNATE);
	LL_GPIO_SetPinSpeed(GPIOA, LL_GPIO_PIN_10, LL_GPIO_SPEED_FREQ_LOW);
	LL_GPIO_SetPinOutputType(GPIOA, LL_GPIO_PIN_10, LL_GPIO_OUTPUT_OPENDRAIN);
	LL_GPIO_SetPinPull(GPIOA, LL_GPIO_PIN_10, LL_GPIO_PULL_UP);
	LL_GPIO_SetAFPin_8_15(GPIOA, LL_GPIO_PIN_10, LL_GPIO_AF_8);
	// Set I2C1 SCL on PA7
	LL_GPIO_SetPinMode(GPIOA, LL_GPIO_PIN_7, LL_GPIO_MODE_ALTERNATE);
	LL_GPIO_SetPinSpeed(GPIOA, LL_GPIO_PIN_7, LL_GPIO_SPEED_FREQ_LOW);
	LL_GPIO_SetPinOutputType(GPIOA, LL_GPIO_PIN_7, LL_GPIO_OUTPUT_OPENDRAIN);
	LL_GPIO_SetPinPull(GPIOA, LL_GPIO_PIN_7, LL_GPIO_PULL_UP);
	LL_GPIO_SetAFPin_0_7(GPIOA, LL_GPIO_PIN_7, LL_GPIO_AF_6);
	// SPI GPIOs
	LL_IOP_GRP1_EnableClock(LL_IOP_GRP1_PERIPH_GPIOA);
	// Configure GPIOs
	// Set SPI1 MISO on PA6
	LL_GPIO_SetPinMode(GPIOA, LL_GPIO_PIN_6, LL_GPIO_MODE_ALTERNATE);
	LL_GPIO_SetPinSpeed(GPIOA, LL_GPIO_PIN_6, LL_GPIO_SPEED_FREQ_LOW);
	LL_GPIO_SetPinOutputType(GPIOA, LL_GPIO_PIN_6, LL_GPIO_OUTPUT_PUSHPULL);
	LL_GPIO_SetPinPull(GPIOA, LL_GPIO_PIN_6, LL_GPIO_PULL_DOWN);
	LL_GPIO_SetAFPin_0_7(GPIOA, LL_GPIO_PIN_6, LL_GPIO_AF_0);
	// Set SPI1 MOSI on PA2
	LL_GPIO_SetPinMode(GPIOA, LL_GPIO_PIN_2, LL_GPIO_MODE_ALTERNATE);
	LL_GPIO_SetPinSpeed(GPIOA, LL_GPIO_PIN_2, LL_GPIO_SPEED_FREQ_LOW);
	LL_GPIO_SetPinOutputType(GPIOA, LL_GPIO_PIN_2, LL_GPIO_OUTPUT_PUSHPULL);
	LL_GPIO_SetPinPull(GPIOA, LL_GPIO_PIN_2, LL_GPIO_PULL_DOWN);
	LL_GPIO_SetAFPin_0_7(GPIOA, LL_GPIO_PIN_2, LL_GPIO_AF_0);
	// Set SPI1 SCK on PA1
	LL_GPIO_SetPinMode(GPIOA, LL_GPIO_PIN_1, LL_GPIO_MODE_ALTERNATE);
	LL_GPIO_SetPinSpeed(GPIOA, LL_GPIO_PIN_1, LL_GPIO_SPEED_FREQ_LOW);
	LL_GPIO_SetPinOutputType(GPIOA, LL_GPIO_PIN_1, LL_GPIO_OUTPUT_PUSHPULL);
	LL_GPIO_SetPinPull(GPIOA, LL_GPIO_PIN_1, LL_GPIO_PULL_NO);
	LL_GPIO_SetAFPin_0_7(GPIOA, LL_GPIO_PIN_1, LL_GPIO_AF_0);
	// UART GPIOs
	LL_IOP_GRP1_EnableClock(LL_IOP_GRP1_PERIPH_GPIOA);
	LL_IOP_GRP1_EnableClock(LL_IOP_GRP1_PERIPH_GPIOB);
	// Configure GPIOs
	// Set UART1 RX on PB2
	LL_GPIO_SetPinMode(GPIOB, LL_GPIO_PIN_2, LL_GPIO_MODE_ALTERNATE);
	LL_GPIO_SetPinSpeed(GPIOB, LL_GPIO_PIN_2, LL_GPIO_SPEED_FREQ_LOW);
	LL_GPIO_SetPinOutputType(GPIOB, LL_GPIO_PIN_2, LL_GPIO_OUTPUT_PUSHPULL);
	LL_GPIO_SetPinPull(GPIOB, LL_GPIO_PIN_2, LL_GPIO_PULL_UP);
	LL_GPIO_SetAFPin_0_7(GPIOB, LL_GPIO_PIN_2, LL_GPIO_AF_0);
	// Set UART1 TX on PA0
	LL_GPIO_SetPinMode(GPIOA, LL_GPIO_PIN_0, LL_GPIO_MODE_ALTERNATE);
	LL_GPIO_SetPinSpeed(GPIOA, LL_GPIO_PIN_0, LL_GPIO_SPEED_FREQ_LOW);
	LL_GPIO_SetPinOutputType(GPIOA, LL_GPIO_PIN_0, LL_GPIO_OUTPUT_PUSHPULL);
	LL_GPIO_SetPinPull(GPIOA, LL_GPIO_PIN_0, LL_GPIO_PULL_NO);
	LL_GPIO_SetAFPin_0_7(GPIOA, LL_GPIO_PIN_0, LL_GPIO_AF_4);

	// Initialize GPIOs
	// Outputs
	ledRed.Init({.mode = GPIO::Mode::Output, .type = GPIO::Output::PushPull, .pull = GPIO::Pull::NoPull});
	ledBlue.Init({.mode = GPIO::Mode::Output, .type = GPIO::Output::PushPull, .pull = GPIO::Pull::NoPull});
	ledGreen.Init({.mode = GPIO::Mode::Output, .type = GPIO::Output::PushPull, .pull = GPIO::Pull::NoPull});
	csX.Init({.mode = GPIO::Mode::Output, .type = GPIO::Output::PushPull, .pull = GPIO::Pull::NoPull});
	drvEnX.Init({.mode = GPIO::Mode::Output, .type = GPIO::Output::PushPull, .pull = GPIO::Pull::NoPull});
	csY.Init({.mode = GPIO::Mode::Output, .type = GPIO::Output::PushPull, .pull = GPIO::Pull::NoPull});
	drvEnY.Init({.mode = GPIO::Mode::Output, .type = GPIO::Output::PushPull, .pull = GPIO::Pull::NoPull});
	csZ.Init({.mode = GPIO::Mode::Output, .type = GPIO::Output::PushPull, .pull = GPIO::Pull::NoPull});
	drvEnZ.Init({.mode = GPIO::Mode::Output, .type = GPIO::Output::PushPull, .pull = GPIO::Pull::NoPull});

	// Inputs
	userButton.Init({.mode = GPIO::Mode::Input, .type = GPIO::Output::PushPull, .pull = GPIO::Pull::PullUp});
	faultX.Init({.mode = GPIO::Mode::Input, .type = GPIO::Output::PushPull, .pull = GPIO::Pull::PullUp});
	faultY.Init({.mode = GPIO::Mode::Input, .type = GPIO::Output::PushPull, .pull = GPIO::Pull::PullUp});
	faultZ.Init({.mode = GPIO::Mode::Input, .type = GPIO::Output::PushPull, .pull = GPIO::Pull::PullUp});

	ledRed.Write(1);
	ledGreen.Write(1);
	ledBlue.Write(1);
	csX.Write(1);
	drvEnX.Write(0);
	csY.Write(1);
	drvEnY.Write(0);
	csZ.Write(1);
	drvEnZ.Write(0);

	// Initialize UARTs
	uart1.Init({.sourceClockHz = 48000000, .baudrate = 115200, .dataBits = UART::DataBits::DataBits_8, .stopBits = UART::StopBits::StopBits_1, .parity = UART::Parity::None, .hwFlowControl = false});

	// Initialize I2Cs
	i2c1.Init({.sourceClockHz = 48000000, .mode = I2C::Mode::Fast, .slaveAddr = 0x00, .EventCallback = I2C1EventHandler, .callbackContext = nullptr});
	i2c2.Init({.sourceClockHz = 48000000, .mode = I2C::Mode::Fast, .slaveAddr = 0x00, .EventCallback = nullptr, .callbackContext = nullptr});

	// Initialize SPIs
	spi1.Init({.sourceClockHz = 48000000, .baudrate = 1000000, .polarity = SPI::ClockPolarity::Low, .phase = SPI::ClockPhase::SecondEdge, .bitOrder = SPI::BitOrder::MSBFirst});
}