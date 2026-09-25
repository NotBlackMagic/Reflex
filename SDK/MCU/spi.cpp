/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright (c) 2026 NotBlackMagic (PlumaLabs)
 *
 * File:    SDK/MCU/spi.cpp
 */

#include "spi.hpp"

#define SPI_USE_IRQ

SPI::SPI(SPI_TypeDef *instance) {
	this->instance = instance;
	this->irqPriority = 0x0D; 	//Lowest priority
	this->isInitialized = false;
	this->isBusy = false;
    this->eventFlags = 0;
	this->txBuffer = nullptr;
	this->txLength = 0;
	this->rxBuffer = nullptr;
	this->rxLength = 0;
}

Status SPI::Init(const Config &config) {
	if(config.baudrate == 0 || config.sourceClockHz == 0) {
		return Status::Error;
	}

	if(this->isInitialized == true) {
		return Status::Ok;
	}

	// Enable bus clocks
	uint32_t fifoThreshold = LL_SPI_RX_FIFO_TH_QUARTER; // Default safe fallback
	if(this->instance == SPI1) {
		LL_APB2_GRP1_EnableClock(LL_APB2_GRP1_PERIPH_SPI1);
		this->irqCall = SPI1_IRQn;
		fifoThreshold = LL_SPI_RX_FIFO_TH_QUARTER;
	}
	else if(this->instance == SPI2) {
		LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_SPI2);
		this->irqCall = SPI2_IRQn;
		fifoThreshold = LL_SPI_RX_FIFO_TH_QUARTER;
	}
	else {
		return Status::Error;
	}
	this->sourceClockHz = config.sourceClockHz;

	// Configure SPI Interface
	this->SetBaudrate(config.baudrate);
	LL_SPI_SetTransferDirection(this->instance, LL_SPI_FULL_DUPLEX);
	LL_SPI_SetClockPhase(this->instance, static_cast<uint32_t>(config.phase));
	LL_SPI_SetClockPolarity(this->instance, static_cast<uint32_t>(config.polarity));
	LL_SPI_SetTransferBitOrder(this->instance, static_cast<uint32_t>(config.bitOrder));
	LL_SPI_SetDataWidth(this->instance, LL_SPI_DATAWIDTH_8BIT);
	LL_SPI_SetRxFIFOThreshold(this->instance, fifoThreshold);	// Set FIFO Length, SPI2_FIFO_LENGTH*Datawidth aka SPI2_FIFO_LENGTH*8bits
	LL_SPI_SetNSSMode(this->instance, LL_SPI_NSS_HARD_OUTPUT);
//	LL_SPI_SetInternalSSLevel(this->instance, LL_SPI_SS_LEVEL_HIGH);					// VERY IMPORTANT, DOES NOT WORK WITHOUT WITH NSS_SOFT!!
//	LL_SPI_EnableNSSPulseMgt(this->instance);
//	LL_SPI_DisableCRC(this->instance);
//	LL_I2S_Disable(this->instance);
	LL_SPI_SetMode(this->instance, LL_SPI_MODE_MASTER);

#ifdef SPI_USE_IRQ
	// Configure SPI Interrupts
	NVIC_SetPriority(this->irqCall, this->irqPriority);
	NVIC_EnableIRQ(this->irqCall);
#endif

	// Enable SPI
	// LL_SPI_Enable(this->instance);

	this->isInitialized = true;
	return Status::Ok;
}

Status SPI::SetBaudrate(uint32_t baudrate) {
	if(baudrate == 0) {
		return Status::Error;
	}

	if(this->isBusy == true) {
		return Status::Error;
	}

	// Calculate best prescaler value, equal or lower then asked frequency
	uint32_t prescaler = 0;				// As power of 2: 0: DIV2, 1: DIV4, ...
	if(baudrate > 0) {
		while(prescaler < 7) {
			uint32_t currentFreq = this->sourceClockHz / (0x01 << (prescaler + 1));
			if(currentFreq <= baudrate) {
				break;
			}
			prescaler += 1;
		}
	}
	else {
		prescaler = 7;
	}

	// Disable SPI to update prescaler
	LL_SPI_Disable(this->instance);
	LL_SPI_SetBaudRatePrescaler(this->instance, ((prescaler) << SPI_CR1_BR_Pos));

	return Status::Ok;
}

Status SPI::TransferAsync(const uint8_t *txBuf, uint8_t *rxBuf, uint32_t len) {
	if(this->isBusy == true) {
		return Status::Busy;
	}

	this->isBusy = true;
    this->eventFlags = 0;

	// Prepare internal transfer variables
	this->txBuffer = txBuf;
	this->txLength = len;
	this->rxBuffer = rxBuf;
	this->rxLength = len;

	// SPI Setup
	LL_SPI_Enable(this->instance);

	// Enable interrupts
	LL_SPI_EnableIT_ERR(this->instance);
	LL_SPI_EnableIT_RXNE(this->instance);
	LL_SPI_EnableIT_TXE(this->instance);

	return Status::Ok;
}

Status SPI::TransferWait(uint32_t timeoutTicks) {
	// Wait for event
	while(this->eventFlags == 0 && timeoutTicks > 0) {
		timeoutTicks--;
	}

	if(this->eventFlags == 0) {
		this->TransferAbort();
		return Status::Timeout;
	}

	if((this->eventFlags & EVT_ERR) == EVT_ERR) {
		// Transfer error occurred
		this->isBusy = false;
		return Status::Error;
	}

	this->isBusy = false;
	return Status::Ok;
}

Status SPI::TransferAbort() {
	// Disable interrupts
#ifdef SPI_USE_IRQ
	LL_SPI_DisableIT_TXE(this->instance);
	LL_SPI_DisableIT_RXNE(this->instance);
	LL_SPI_DisableIT_ERR(this->instance);
#endif

	// Disable peripheral
	LL_SPI_Disable(this->instance);

	// Clear transfer context
	this->txBuffer = nullptr;
	this->txLength = 0;
	this->rxBuffer = nullptr;
	this->rxLength = 0;

	// Signal event flags and release bus
	this->eventFlags |= EVT_ERR;
	this->isBusy = false;

	return Status::Ok;
}

// ---------------------------------------------------------
// IRQ Handler
// ---------------------------------------------------------

void SPI::InterruptHandler() {
	// Handle Errors (Overrun, Mode Fault, etc)
	if(LL_SPI_IsActiveFlag_OVR(this->instance) == 0x01 || LL_SPI_IsActiveFlag_MODF(this->instance) == 0x01) {
		// Clear flags
		LL_SPI_ClearFlag_OVR(this->instance);
		LL_SPI_ClearFlag_MODF(this->instance);

		LL_SPI_DisableIT_TXE(this->instance);
		LL_SPI_DisableIT_RXNE(this->instance);
		LL_SPI_DisableIT_ERR(this->instance);

		// Signal completion (replaces tx_event_flags_set)
		this->eventFlags |= EVT_ERR;
		return;
	}

	// Read data from RX FIFO until empty: RXP means at least ONE packet (one FIFO Threshold) can be read
	while(LL_SPI_IsEnabledIT_RXNE(this->instance) == 0x01 && LL_SPI_IsActiveFlag_RXNE(this->instance) == 0x01 && this->rxLength > 0) {
		uint8_t rxByte = LL_SPI_ReceiveData8(this->instance);
		if(this->rxBuffer != nullptr) {
			*this->rxBuffer = rxByte;
			this->rxBuffer = this->rxBuffer + 1;
		}
		this->rxLength = this->rxLength - 1;
	}

	// Write data to TX FIFO until is full: TXP means at least ONE packet (one FIFO Threshold) can be written
	while(LL_SPI_IsEnabledIT_TXE(this->instance) == 0x01 && LL_SPI_IsActiveFlag_TXE(this->instance) == 0x01) {
		if(this->txLength > 0) {
			uint8_t txByte = (this->txBuffer != nullptr) ? *this->txBuffer : 0xFF;
			LL_SPI_TransmitData8(this->instance, txByte);
			if(this->txBuffer != nullptr) {
				this->txBuffer = this->txBuffer + 1;
			}
			this->txLength = this->txLength - 1;
		}
		else {
			LL_SPI_DisableIT_TXE(this->instance);
        }
	}

	// Handle End of Transfer
	if(this->txLength == 0 && this->rxLength == 0) {
		// Wait for the busy flag to clear before disabling the peripheral
		if(LL_SPI_IsActiveFlag_BSY(this->instance) == 0) {
			LL_SPI_DisableIT_RXNE(this->instance);
			LL_SPI_DisableIT_ERR(this->instance);
			LL_SPI_Disable(this->instance);

			// Signal completion
			this->eventFlags |= EVT_TRANS_CPLT;
		}
	}
}