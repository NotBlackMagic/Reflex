/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright (c) 2026 NotBlackMagic (PlumaLabs)
 *
 * File:    SDK/MCU/system.hpp
 * Author:  NotBlackMagic
 * Brief:   XXX
 */

#pragma once

#include <stdint.h>

#include "stm32c0xx.h"
#include "stm32c0xx_ll_bus.h"
#include "stm32c0xx_ll_cortex.h"
#include "stm32c0xx_ll_exti.h"
#include "stm32c0xx_ll_gpio.h"
#include "stm32c0xx_ll_pwr.h"
#include "stm32c0xx_ll_rcc.h"
#include "stm32c0xx_ll_system.h"
#include "stm32c0xx_ll_utils.h"

struct System {
	// Delete constructor.
	System() = delete;

	/// @brief Initializes the overall system clock tree.
	static void InitClock(void);

	/// @brief Initializes the SysTick timer.
	/// @note Typically used by the RTOS.
	static void InitSysTick(void);

	/// @brief Disables the SysTick timer.
	static void DisableSysTick(void);

	/// @brief Resets the MCU.
	static void Reset();
};

struct Time {
	// Delete constructor.
	Time() = delete;

	/// @brief Initializes the basic system timer (TIM5).
	static void Init();

	/// @brief Disables the basic system timer (TIM5).
	static void Disable(void);

	/// @brief Gets the current time since boot in miliseconds.
	static uint32_t GetMs();

	/// @brief Gets the current time since boot in microseconds.
	static uint64_t GetUs();

	/// @brief Delays/blocks for a set amount of time (milliseconds).
	/// @param ms Milliseconds to wait.
	static void Delay(uint32_t ms);

	/// @brief Delays/blocks for a set amount of NOP instructions.
	/// @param count: Number of NOPs to execute
	static void DelayNOP(uint32_t count);
};