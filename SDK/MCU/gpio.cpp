/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright (c) 2026 NotBlackMagic (PlumaLabs)
 *
 * File:    SDK/MCU/gpio.cpp
 */

#include "gpio.hpp"

GPIO::GPIO(GPIO_TypeDef *port, uint32_t pin) {
	this->port = port;
	this->pin = pin;
	this->irqPriority = 0x0F; // Lowest priority

	// Find and map corresponding EXTI port
	if(this->port == GPIOA)	{
		this->extiPort = LL_EXTI_CONFIG_PORTA;
	}
	else if(this->port == GPIOB) {
		this->extiPort = LL_EXTI_CONFIG_PORTB;
	}
	else if(this->port == GPIOC) {
		this->extiPort = LL_EXTI_CONFIG_PORTC;
	}
	else if(this->port == GPIOD) {
		this->extiPort = LL_EXTI_CONFIG_PORTD;
	}
	else if(this->port == GPIOF) {
		this->extiPort = LL_EXTI_CONFIG_PORTF;
	}

	// Find and map corresponding EXTI line
	if(this->pin == LL_GPIO_PIN_0) {
		this->extiLine = LL_EXTI_LINE_0;
		this->extiExtiLine = LL_EXTI_CONFIG_LINE0;
		this->irqCall = EXTI0_1_IRQn;
	}
	else if(this->pin == LL_GPIO_PIN_1) {
		this->extiLine = LL_EXTI_LINE_1;
		this->extiExtiLine = LL_EXTI_CONFIG_LINE1;
		this->irqCall = EXTI0_1_IRQn;
	}
	else if(this->pin == LL_GPIO_PIN_2) {
		this->extiLine = LL_EXTI_LINE_2;
		this->extiExtiLine = LL_EXTI_CONFIG_LINE2;
		this->irqCall = EXTI2_3_IRQn;
	}
	else if(this->pin == LL_GPIO_PIN_3) {
		this->extiLine = LL_EXTI_LINE_3;
		this->extiExtiLine = LL_EXTI_CONFIG_LINE3;
		this->irqCall = EXTI2_3_IRQn;
	}
	else if(this->pin == LL_GPIO_PIN_4) {
		this->extiLine = LL_EXTI_LINE_4;
		this->extiExtiLine = LL_EXTI_CONFIG_LINE4;
		this->irqCall = EXTI4_15_IRQn;
	}
	else if(this->pin == LL_GPIO_PIN_5) {
		this->extiLine = LL_EXTI_LINE_5;
		this->extiExtiLine = LL_EXTI_CONFIG_LINE5;
		this->irqCall = EXTI4_15_IRQn;
	}
	else if(this->pin == LL_GPIO_PIN_6) {
		this->extiLine = LL_EXTI_LINE_6;
		this->extiExtiLine = LL_EXTI_CONFIG_LINE6;
		this->irqCall = EXTI4_15_IRQn;
	}
	else if(this->pin == LL_GPIO_PIN_7) {
		this->extiLine = LL_EXTI_LINE_7;
		this->extiExtiLine = LL_EXTI_CONFIG_LINE7;
		this->irqCall = EXTI4_15_IRQn;
	}
	else if(this->pin == LL_GPIO_PIN_8) {
		this->extiLine = LL_EXTI_LINE_8;
		this->extiExtiLine = LL_EXTI_CONFIG_LINE8;
		this->irqCall = EXTI4_15_IRQn;
	}
	else if(this->pin == LL_GPIO_PIN_9) {
		this->extiLine = LL_EXTI_LINE_9;
		this->extiExtiLine = LL_EXTI_CONFIG_LINE9;
		this->irqCall = EXTI4_15_IRQn;
	}
	else if(this->pin == LL_GPIO_PIN_10) {
		this->extiLine = LL_EXTI_LINE_10;
		this->extiExtiLine = LL_EXTI_CONFIG_LINE10;
		this->irqCall = EXTI4_15_IRQn;
	}
	else if(this->pin == LL_GPIO_PIN_11) {
		this->extiLine = LL_EXTI_LINE_11;
		this->extiExtiLine = LL_EXTI_CONFIG_LINE11;
		this->irqCall = EXTI4_15_IRQn;
	}
	else if(this->pin == LL_GPIO_PIN_12) {
		this->extiLine = LL_EXTI_LINE_12;
		this->extiExtiLine = LL_EXTI_CONFIG_LINE12;
		this->irqCall = EXTI4_15_IRQn;
	}
	else if(this->pin == LL_GPIO_PIN_13) {
		this->extiLine = LL_EXTI_LINE_13;
		this->extiExtiLine = LL_EXTI_CONFIG_LINE13;
		this->irqCall = EXTI4_15_IRQn;
	}
	else if(this->pin == LL_GPIO_PIN_14) {
		this->extiLine = LL_EXTI_LINE_14;
		this->extiExtiLine = LL_EXTI_CONFIG_LINE14;
		this->irqCall = EXTI4_15_IRQn;
	}
	else if(this->pin == LL_GPIO_PIN_15) {
		this->extiLine = LL_EXTI_LINE_15;
		this->extiExtiLine = LL_EXTI_CONFIG_LINE15;
		this->irqCall = EXTI4_15_IRQn;
	}
}

void GPIO::Init(const Config &config) {
	LL_GPIO_SetPinSpeed(this->port, this->pin, LL_GPIO_SPEED_FREQ_LOW);
	LL_GPIO_SetPinOutputType(this->port, this->pin, static_cast<uint32_t>(config.type));
	LL_GPIO_SetPinPull(this->port, this->pin, static_cast<uint32_t>(config.pull));
	LL_GPIO_SetPinMode(this->port, this->pin, static_cast<uint32_t>(config.mode));
}

void GPIO::Write(uint8_t level) {
	if(level == 0) {
		LL_GPIO_ResetOutputPin(this->port, this->pin);
	} 
	else {
		LL_GPIO_SetOutputPin(this->port, this->pin);
	}
}

uint8_t GPIO::Read() {
	return LL_GPIO_IsInputPinSet(this->port, this->pin);
}

void GPIO::Toggle() {
	LL_GPIO_TogglePin(this->port, this->pin);
}

uint8_t GPIO::GetPinIndex() const {
	// e.g., LL_GPIO_PIN_2 (0x0004) has 2 trailing zeros -> returns 2
	return static_cast<uint8_t>(__builtin_ctz(this->pin)); 
}

void GPIO::EnableIRQ(GPIO::Interrupt trigger, uint8_t irqPriority) {
	// Configure NVIC EXTI Interrupts
	this->irqPriority = irqPriority;

	// Set Input Pins Interrupts
	LL_EXTI_SetEXTISource(this->extiPort, this->extiExtiLine);
	LL_EXTI_DisableEvent_0_31(this->extiLine);
	LL_EXTI_EnableIT_0_31(this->extiLine);
	switch (trigger) {
		case GPIO::Interrupt::None:
			// No trigger
			break;
		case GPIO::Interrupt::Rising:
			LL_EXTI_DisableFallingTrig_0_31(this->extiLine);
			LL_EXTI_EnableRisingTrig_0_31(this->extiLine);
			break;
		case GPIO::Interrupt::Falling:
			LL_EXTI_DisableRisingTrig_0_31(this->extiLine);
			LL_EXTI_EnableFallingTrig_0_31(this->extiLine);
			break;
		case GPIO::Interrupt::Both:
			LL_EXTI_EnableRisingTrig_0_31(this->extiLine);
			LL_EXTI_EnableFallingTrig_0_31(this->extiLine);
			break;
	}

	NVIC_SetPriority(this->irqCall, this->irqPriority);
	NVIC_EnableIRQ(this->irqCall);
}

void GPIO::DisableIRQ() {
	LL_EXTI_DisableIT_0_31(this->extiLine);
	LL_EXTI_DisableRisingTrig_0_31(this->extiLine);
	LL_EXTI_DisableFallingTrig_0_31(this->extiLine);
	NVIC_EnableIRQ(this->irqCall);
}