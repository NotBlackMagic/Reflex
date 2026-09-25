/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright (c) 2026 NotBlackMagic (PlumaLabs)
 *
 * File:    SDK/MCU/i2c.hpp
 * Author:  NotBlackMagic
 * Brief:   I2C dual-role (Master/Slave) driver class for STM32C0 using LL (Low-Layer) API and direct register access (NO HALL).
 */

#pragma once

#include <stdint.h>
#include <string.h>

#include "stm32c0xx.h"
#include "stm32c0xx_ll_bus.h"
#include "stm32c0xx_ll_exti.h"
#include "stm32c0xx_ll_gpio.h"
#include "stm32c0xx_ll_i2c.h"

#include "status.hpp"
#include "system.hpp"

/// @brief Driver for the I2C peripheral in master mode supporting Blocking and Async (Interrupt) modes.
class I2C {
	public:
		/// @brief Defines the I2C bus speed.
		enum class Mode {
			Standard,		///< 100kHz (Standard Mode)
			Fast,			///< 400kHz (Fast Mode)
			FastPlus		///< 1MHz (Fast Mode Plus)
		};

		/// @brief Tracks the current active role of the hardware.
		enum class Role {
			Idle,
			Master,
			Slave
		};

		/// @brief I2C event.
		enum class Event {
			TransferComplete,
			Error
		};

		/// @brief I2C peripheral configuration structure.
		struct Config {
			uint32_t sourceClockHz;	///< Peripheral source clock frequency in Hz
			Mode mode;				///< The target bus speed configuration.
			uint8_t slaveAddr;		///< 7-bit slave address. Set to 0 to disable slave mode.

			void (*EventCallback)(void* ctx, Event event);
			void* callbackContext;
		};

		// Delete copy constructors
		I2C(const I2C&) = delete;
		I2C& operator=(const I2C&) = delete;

		/// @brief Constructor.
		/// @param instance Pointer to the hardware instance (e.g., I2C1, I2C2).
		I2C(I2C_TypeDef *instance);

		/// @brief Initializes the peripheral clock, the peripheral itself, and interrupts.
		/// @param config I2C configuration.
		/// @return Status::Ok if initialization succeeded, or Status::Error if the config was invalid.
		Status Init(const Config &config);

		// ---------------------------------------------------------
		// Master API
		// ---------------------------------------------------------

		/// @brief Checks if a device exists on the bus.
		/// @param addr The 7-bit slave address to check.
		/// @return 1 (True) if the device acknowledges (ACK), 0 if NACK.
		uint8_t Probe(uint16_t addr);
		
		/// @brief Starts a non-blocking transaction (Write, Read, or Write-then-Read).
		/// @note  This function blocks momentarily if the BUS is busy, then returns.
		/// @details Returns immediately. Use TransferWait() to synchronize completion.
		/// @param addr		7-bit slave address.
		/// @param txBuf	Pointer to data to write (need to be kept until end of transfer).
		/// @param txLen	Number of bytes to write.
		/// @param rxBuf	Pointer to buffer for read data.
		/// @param rxLen	Number of bytes to read.
		/// @return Status::Ok if the transfer started, or Status::Busy if the I2C is locked by another thread.
		Status TransferAsync(uint16_t addr, const uint8_t *txBuf, uint16_t txLen, uint8_t *rxBuf, uint16_t rxLen);
		
		/// @brief Blocks the current thread until the Async transfer completes.
		/// @param timeoutTicks Max wait time in OS ticks.
		/// @return Status::Ok if the transfer completed successfully, Status::Timeout if it expired, or Status::Error on hardware faults.
		Status TransferWait(uint32_t timeoutTicks);

		/// @brief Aborts a ongoing transfer.
		/// @return Status::Ok
		Status TransferAbort();

		// ---------------------------------------------------------
		// Slave API
		// ---------------------------------------------------------

		/// @brief Prepares/sets the buffer to be transmitted on a read from a master.
		/// @param buf	Pointer to data to be transferred (need to be kept until end of transfer).
		/// @param len	Number of bytes to be transferred.
		/// @return Status::Ok
		Status SetSlaveTX(const uint8_t *buf, uint16_t len);

		/// @brief Prepares/sets the buffer to save the written data from a master.
		/// @param buf	Pointer to buffer to be filled with data from master.
		/// @param len	Number of bytes to be transferred.
		/// @return Status::Ok
		Status SetSlaveRX(uint8_t *buf, uint16_t len);

		/// @brief Interrupt Service Routine handler.
		/// @warning This function is called by the NVIC. Do not call manually.
		void InterruptHandler();

	private:
		I2C_TypeDef *instance;
		IRQn_Type irqCall;
		uint8_t irqPriority;
		Config config;
		bool isInitialized;

		// State Tracking
		volatile Role role;
		volatile bool isBusy;
		volatile uint32_t eventFlags;

		// Master Context
		uint8_t address;
		const uint8_t * volatile txBuffer;
		volatile uint16_t txLength;
		uint8_t * volatile rxBuffer;
		volatile uint16_t rxLength;

		// Slave Context
		const uint8_t * slaveTXBuf;
		uint16_t slaveTXLen;
		volatile uint16_t slaveTXIdx;
		uint8_t * slaveRXBuf;
		uint16_t slaveRXLen;
		volatile uint16_t slaveRXIdx;

		// Event Flags Definitions
		static constexpr uint32_t EVT_TRANS_CPLT = 0x01;
		static constexpr uint32_t EVT_ERR = 0x02;

		// Timeout defines
		static constexpr uint32_t TIMEOUT_BUSY_US = 25;
};