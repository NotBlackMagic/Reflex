/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright (c) 2026 NotBlackMagic (PlumaLabs)
 *
 * File:    SDK/MCU/system.cpp
 */

#include "system.hpp"

// ============================================================================
// System implementations
// ============================================================================

void System::InitClock(void) {
	// Reset of all peripherals, Initializes the Flash interface and the Systick
	LL_APB2_GRP1_EnableClock(LL_APB2_GRP1_PERIPH_SYSCFG);
	LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_PWR);

	// Set Flash Latency
	LL_FLASH_SetLatency(LL_FLASH_LATENCY_1);

	// Enable External Clock in on HSI
	LL_RCC_HSI_Enable();
	while(LL_RCC_HSI_IsReady() != 1);		//Wait till HSI is ready
	LL_RCC_HSI_SetCalibTrimming(64);
	LL_RCC_SetHSIDiv(LL_RCC_HSI_DIV_1);

	// Set AHB prescaler
	LL_RCC_SetAHBPrescaler(LL_RCC_HCLK_DIV_1);
	LL_RCC_SetSYSDivider(LL_RCC_SYSCLK_DIV_1);

	// Sysclk activation on the HSI
	LL_RCC_SetSysClkSource(LL_RCC_SYS_CLKSOURCE_HSI);
	while(LL_RCC_GetSysClkSource() != LL_RCC_SYS_CLKSOURCE_STATUS_HSI);

	// Set APB1 prescaler
	LL_RCC_SetAPB1Prescaler(LL_RCC_APB1_DIV_1);

	// Update CMSIS variable (which can be updated also through SystemCoreClockUpdate function)
	LL_SetSystemCoreClock(48000000);

	LL_RCC_SetI2CClockSource(LL_RCC_I2C1_CLKSOURCE_PCLK1);
	LL_RCC_SetI2SClockSource(LL_RCC_I2S1_CLKSOURCE_SYSCLK);
	LL_RCC_SetUSARTClockSource(LL_RCC_USART1_CLKSOURCE_PCLK1);
}

void System::InitSysTick(void) {
	LL_Init1msTick(SystemCoreClock);
	LL_SYSTICK_EnableIT();
}

void System::DisableSysTick(void) {
	LL_SYSTICK_DisableIT();
}

void System::Reset() {

}

// ============================================================================
// Time implementations (using TIM5)
// ============================================================================

static volatile uint32_t systickCnt = 0;
static volatile uint32_t timeMills = 0;
static volatile uint32_t timeHours = 0;

// void Time::Init() {
// 	// Enable bus clocks 
// 	LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_TIM5);

// 	// Configure Timer
// 	uint32_t periphClock = 400000000;					// TIM5 is connected to "Timer Group Clocks" i.e. to SYSB through TIMPRE
// 	uint32_t prescaler = (periphClock / 1000000) - 1;	// Set Clock to 1MHz -> 1us period
// 	uint32_t arr = (3600000000UL - 1);					// Set update period to 1h
// 	LL_TIM_SetPrescaler(TIM5, prescaler);
// 	LL_TIM_SetCounterMode(TIM5, LL_TIM_COUNTERMODE_UP);
// 	LL_TIM_SetAutoReload(TIM5, arr);
// 	LL_TIM_EnableARRPreload(TIM5);

// 	// This is needed to update the prescaler
// 	LL_TIM_GenerateEvent_UPDATE(TIM5);

// 	// Reset timer counters
// 	timeMills = 0;
// 	timeHours = 0;

// 	// Configure Interrupts
// 	NVIC_SetPriority(TIM5_IRQn, 0);
// 	NVIC_EnableIRQ(TIM5_IRQn);
// 	LL_TIM_EnableIT_UPDATE(TIM5);

// 	// Enable Timer
// 	LL_TIM_EnableCounter(TIM5);
// }

// void Time::Disable(void) {
// 	LL_TIM_DisableCounter(TIM5);
// 	LL_TIM_DisableIT_UPDATE(TIM5);
// 	NVIC_DisableIRQ(TIM5_IRQn);
// 	NVIC_ClearPendingIRQ(TIM5_IRQn);
// }

uint32_t Time::GetMs() {
	return (uint32_t)(GetUs() / 1000);
}

uint64_t Time::GetUs() {
	// uint32_t cnt = LL_TIM_GetCounter(TIM5);
	// return ((uint64_t)timeHours * 3600000000ULL) + cnt;
	// // return ((timeMills * 1000) + LL_TIM_GetCounter(TIM5));
	return systickCnt * 1000;
}

void Time::Delay(uint32_t ms) {
	uint64_t start = GetUs();
	while ((GetUs() - start) < ((uint64_t)ms * 1000));
}

void Time::DelayNOP(uint32_t count) {
	volatile uint32_t i;
	for(i = 0; i < count; i++) {
		__NOP();
	}
}

// ---------------------------------------------------------
// IRQ Handler
// ---------------------------------------------------------

/**
  * @brief This function handles System tick timer.
  */
extern "C" {
	void SysTick_Handler(void) {
		systickCnt += 1;
	}
}

// extern "C" {
// 	void TIM5_IRQHandler(void) {
// 		// timeMills += 1;
// 		timeHours += 1;
// 		LL_TIM_ClearFlag_UPDATE(TIM5);
// 	}
// }