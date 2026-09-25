/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright (c) 2026 NotBlackMagic (PlumaLabs)
 *
 * File:    SDK/MCU/i2c.cpp
 */

//Reserved I2C Addresses List (https://www.ti.com/content/dam/videos/external-videos/en-us/8/3816841626001/6241024036001.mp4/subassets/adcs-introduction-to-i2c-reserved-addresses-presentation.pdf)
//0x00: (R/Wn = 0) General call address
//0x00: (R/Wn = 1) Start byte
//0x01: (R/Wn = X) CBUS address
//0x02: (R/Wn = X) Reserved for different bus format
//0x03: (R/Wn = X) Reserved for future purposes
//0x04-0x07: (R/Wn = X) HS-mode controller code
//0x78-0x7B: (R/Wn = X) 10-bit target addressing
//0x7C-0x7F: (R/Wn = 1) Device ID

#include "i2c.hpp"

I2C::I2C(I2C_TypeDef *instance) {
	this->instance = instance;
	this->irqPriority = 0x0E; // Lowest priority (safe default)
	this->isInitialized = false;
	this->isBusy = false;
	this->role = Role::Idle;
	this->eventFlags = 0;
	// Master context
	this->address = 0;
	this->txBuffer = nullptr;
	this->txLength = 0;
	this->rxBuffer = nullptr;
	this->rxLength = 0;
	// Slave context
	this->slaveTXBuf = nullptr;
	this->slaveTXLen = 0;
	this->slaveTXIdx = 0;
	this->slaveRXBuf = nullptr;
	this->slaveRXLen = 0;
	this->slaveRXIdx = 0;
}

Status I2C::Init(const Config &config) {
	if(this->isInitialized == true) {
		return Status::Ok;
	}

	this->config = config;

	// Enable bus clocks and identify IRQ lines
	if(this->instance == I2C1) {
		LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_I2C1);
		this->irqCall = I2C1_IRQn;
	}
	else if(this->instance == I2C2) {
		LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_I2C2);
		this->irqCall = I2C2_IRQn;
	}
	else {
		return Status::Error;
	}

	// Configure I2C Interface
	// Configure the SDA setup, hold time and the SCL high, low period
	switch (config.mode) {
		case I2C::Mode::Standard:
			//I2C Standard Mode (100kHz)
			LL_I2C_SetTiming(this->instance, 0x10805D88);		//48MHz Clock
			break;
		case I2C::Mode::Fast:
			//I2C Fast Mode (400kHz)
			LL_I2C_SetTiming(this->instance, 0x0090194B);		//48MHz Clock
			break;
		case I2C::Mode::FastPlus:
			//I2C Fast Mode Plus (1MHz)
			LL_I2C_SetTiming(this->instance, 0x00700814);		//48MHz Clock
			break;
	}

	// Configure Slave Address if requested
	if(config.slaveAddr != 0) {
		LL_I2C_SetOwnAddress1(this->instance, config.slaveAddr << 1, LL_I2C_OWNADDRESS1_7BIT);
		LL_I2C_EnableOwnAddress1(this->instance);
		LL_I2C_EnableIT_ADDR(this->instance); // Enable Address Match Interrupt
	}

//	LL_I2C_SetOwnAddress2(this->instance, 0x00, LL_I2C_OWNADDRESS2_NOMASK);	//Reset Values of: OwnAddress2 is 0x00; OwnAddrMask is LL_I2C_OWNADDRESS2_NOMASK
//	LL_I2C_DisableOwnAddress2(this->instance);								//Reset Value is Own Address2 is disabled
//	LL_I2C_EnableClockStretching(this->instance);							//Reset Value is Clock stretching enabled
//	LL_I2C_SetDigitalFilter(this->instance, 0x00);							//Reset Value is 0x00
//	LL_I2C_EnableAnalogFilter(this->instance);								//Reset Value is Analog Filter enabled
//	LL_I2C_EnableGeneralCall(this->instance);								//Reset Value is General Call disabled
//	LL_I2C_SetMasterAddressingMode(this->instance, LL_I2C_ADDRESSING_MODE_7BIT);	//Reset Value is LL_I2C_ADDRESSING_MODE_7BIT
//	LL_I2C_SetMode(this->instance, LL_I2C_MODE_I2C);						//Reset Value is I2C mode

	// Configure Interrupts
	NVIC_SetPriority(this->irqCall, this->irqPriority);
	NVIC_EnableIRQ(this->irqCall);
//	NVIC_SetPriority(I2C1_ER_IRQn, 1);
//	NVIC_EnableIRQ(I2C1_ER_IRQn);
	LL_I2C_EnableIT_RX(this->instance);
	LL_I2C_EnableIT_NACK(this->instance);
	LL_I2C_EnableIT_STOP(this->instance);
//	LL_I2C_EnableIT_ERR(this->instance);

	// Enable I2C
	LL_I2C_Enable(this->instance);

	this->isInitialized = true;
	return Status::Ok;
}

uint8_t I2C::Probe(uint16_t addr) {
	if(this->isBusy == true || this->role == Role::Slave) {
		return 0x00;
	}

	this->isBusy = true;
	this->role = Role::Master;

	uint64_t timestamp = Time::GetUs();
	while(true) {
		if(LL_I2C_IsActiveFlag_BUSY(this->instance) == 0x00) {
			break;
		}

		if((Time::GetUs() - timestamp) > TIMEOUT_BUSY_US) {
			// Wait for Busy timeout
			this->isBusy = false;
			return 0x00;
		}
	}

	// Clear event flags
	this->eventFlags = 0;

	this->address = addr << 1;
	this->txLength = 0;
	this->rxLength = 0;

	// Enable Interrupts
	LL_I2C_EnableIT_NACK(this->instance);
	LL_I2C_EnableIT_STOP(this->instance);

	// Start a 0-byte Write to check for ACK
	LL_I2C_HandleTransfer(this->instance, this->address, LL_I2C_ADDRSLAVE_7BIT, this->txLength, LL_I2C_MODE_AUTOEND, LL_I2C_GENERATE_START_WRITE);

	// Wait for event (STOP or NACK)
	while(this->eventFlags == 0) {

	}

	uint8_t nack = 0x00;
	if((this->eventFlags & EVT_ERR) == EVT_ERR) {
		nack = 0x01;
	}

	// Disable interrupts again
	LL_I2C_DisableIT_NACK(this->instance);
	LL_I2C_DisableIT_STOP(this->instance);

	// Release I2C device
	this->isBusy = false;
	this->role = Role::Idle;

	return !nack;
}

Status I2C::TransferAsync(uint16_t addr, const uint8_t *txBuf, uint16_t txLen, uint8_t *rxBuf, uint16_t rxLen) {
	// Prevent overflow of I2C internal transfer length register part
	if(txLen > 255 || rxLen > 255) {
		return Status::Error;
	}
	
	if(this->isBusy == true || this->role == Role::Slave || LL_I2C_IsActiveFlag_BUSY(this->instance) == 0x01) {
		return Status::Busy;
	}

	this->role = Role::Master;
	this->isBusy = true;

	// Clear event flags
	this->eventFlags = 0;

	// Prepare internal transfer variables
	this->address = addr << 1;
	this->txBuffer = txBuf;
	this->txLength = txLen;
	this->rxBuffer = rxBuf;
	this->rxLength = rxLen;

	uint32_t transferMode;
	if(this->rxLength > 0 && this->txLength > 0) {
		// TX followed by RX, use Repeated Start (SoftEnd)
		transferMode = LL_I2C_MODE_SOFTEND;
		LL_I2C_EnableIT_TC(this->instance);
	}
	else {
		// Just TX or Just RX, use AutoEnd
		transferMode = LL_I2C_MODE_AUTOEND;
		LL_I2C_DisableIT_TC(this->instance);
	}

	LL_I2C_EnableIT_NACK(this->instance);
	LL_I2C_EnableIT_STOP(this->instance);
	// LL_I2C_EnableIT_ERR(this->instance);

	if(this->txLength > 0) {
		LL_I2C_HandleTransfer(this->instance, this->address, LL_I2C_ADDRSLAVE_7BIT, this->txLength, transferMode, LL_I2C_GENERATE_START_WRITE);
		LL_I2C_EnableIT_TX(this->instance);
	} 
	else if(this->rxLength > 0) {
		LL_I2C_HandleTransfer(this->instance, this->address, LL_I2C_ADDRSLAVE_7BIT, this->rxLength, LL_I2C_MODE_AUTOEND, LL_I2C_GENERATE_START_READ);
		LL_I2C_EnableIT_RX(this->instance);
	}

	return Status::Ok;
}

Status I2C::TransferWait(uint32_t timeoutTicks) {
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
		this->TransferAbort();
		return Status::Error;
	}

	this->isBusy = false;
	return Status::Ok;
}

Status I2C::TransferAbort() {
	// Disable interrupts
	LL_I2C_DisableIT_TX(this->instance);
	LL_I2C_DisableIT_RX(this->instance);
	LL_I2C_DisableIT_STOP(this->instance);
	LL_I2C_DisableIT_NACK(this->instance);
	// LL_I2C_DisableIT_ERR(this->instance);

	// Stop current transfer if any
	if(LL_I2C_IsActiveFlag_BUSY(this->instance) == 0x01) {
		LL_I2C_GenerateStopCondition(this->instance);
	}

	// Disable peripheral
	LL_I2C_Disable(this->instance);

	// Clear transfer context
	this->address = 0;
	this->txBuffer = nullptr;
	this->txLength = 0;
	this->rxBuffer = nullptr;
	this->rxLength = 0;

	// Re-enable peripheral
	LL_I2C_Enable(this->instance);

	// Signal event flags and release bus
	this->eventFlags |= EVT_TRANS_CPLT;
	this->isBusy = false;
	this->role = Role::Idle;

	return Status::Ok;
}

Status I2C::SetSlaveTX(const uint8_t *buf, uint16_t len) {
	this->slaveTXBuf = buf;
	this->slaveTXLen = len;
}

Status I2C::SetSlaveRX(uint8_t *buf, uint16_t len) {
	this->slaveRXBuf = buf;
	this->slaveRXLen = len;
}

// ---------------------------------------------------------
// IRQ Handler
// ---------------------------------------------------------

void I2C::InterruptHandler() {
	// Handle address match (ADDR)
	if(LL_I2C_IsActiveFlag_ADDR(this->instance) == 0x01) {
		this->role = Role::Slave;
		this->slaveTXIdx = 0;
		this->slaveRXIdx = 0;
		
		// Clear flag to acknowledge the address and start clocking data
		LL_I2C_ClearFlag_ADDR(this->instance);
		
		// Ensure TX/RX interrupts are ready
		LL_I2C_EnableIT_TX(this->instance);
		LL_I2C_EnableIT_RX(this->instance);
		return; // Early return to process subsequent flags
	}
	
	// Handle NACK
	if(LL_I2C_IsActiveFlag_NACK(this->instance) == 0x01) {
		// NACK received, stop transaction and flag error
		if(this->role == Role::Master) {
			LL_I2C_GenerateStopCondition(this->instance);
			this->eventFlags |= EVT_ERR;
		}
		LL_I2C_ClearFlag_NACK(this->instance);
	}

	// Handle Transmit (TXIS)
	if(LL_I2C_IsEnabledIT_TX(this->instance) == 0x01 && LL_I2C_IsActiveFlag_TXIS(this->instance) == 0x01) {
		if(this->role == Role::Master) {
			if(this->txLength > 0) {
				LL_I2C_TransmitData8(this->instance, *this->txBuffer);
				this->txBuffer = this->txBuffer + 1;
				this->txLength = this->txLength - 1;
			}
		}
		else if(this->role == Role::Slave) {
			if(this->slaveTXIdx < this->slaveTXLen && this->slaveTXBuf != nullptr) {
				LL_I2C_TransmitData8(this->instance, this->slaveTXBuf[this->slaveTXIdx]);
				this->slaveTXIdx = this->slaveTXIdx + 1;
			}
			else {
				// Requesting more bytes then have, send dummies
				LL_I2C_TransmitData8(this->instance, 0xFF);
			}
		}
		else {
			this->isBusy = false;
			LL_I2C_DisableIT_TX(this->instance);
		}
	}

	// Handle Receive (RXNE)
	if(LL_I2C_IsActiveFlag_RXNE(this->instance) == 0x01) {
		if(this->role == Role::Master) {
			if(this->rxLength > 0) {
				*this->rxBuffer = LL_I2C_ReceiveData8(this->instance);
				this->rxBuffer = this->rxBuffer + 1;
				this->rxLength = this->rxLength - 1;
			}
		}
		else if(this->role == Role::Slave) {
			uint8_t data = LL_I2C_ReceiveData8(this->instance);
			if(this->slaveRXIdx < this->slaveRXLen && this->slaveRXBuf != nullptr) {
				this->slaveRXBuf[this->slaveRXIdx] = data;
				this->slaveRXIdx = this->slaveRXIdx + 1;
			}
		}
	}

	// Handle Transfer Complete (TC)
	if(LL_I2C_IsEnabledIT_TC(this->instance) == 0x01 && LL_I2C_IsActiveFlag_TC(this->instance) == 0x01) {
		if(this->role == Role::Master) {
			if(this->rxLength > 0) {
				// Generate REPEATED START for Reading
				LL_I2C_HandleTransfer(this->instance, this->address, LL_I2C_ADDRSLAVE_7BIT, this->rxLength, LL_I2C_MODE_AUTOEND, LL_I2C_GENERATE_START_READ);
				
				// Switch to RX interrupts
				LL_I2C_DisableIT_TX(this->instance);
				LL_I2C_EnableIT_RX(this->instance);
			}
			else {
				LL_I2C_DisableIT_TC(this->instance);
			}
		}
	}

	// Handle Stop Condition (End of Transaction)
	if(LL_I2C_IsActiveFlag_STOP(this->instance) == 0x01) {
		if(this->role == Role::Master) {
			LL_I2C_DisableIT_TX(this->instance);
			LL_I2C_DisableIT_RX(this->instance);
			this->eventFlags |= EVT_TRANS_CPLT;
			this->isBusy = false;
			this->role = Role::Idle;
			if(this->config.EventCallback != nullptr) {
				this->config.EventCallback(this->config.callbackContext, Event::TransferComplete);
			}
		}
		else if(this->role == Role::Slave) {
			LL_I2C_DisableIT_TX(this->instance);
			this->role = Role::Idle;
			if(this->config.EventCallback != nullptr) {
				this->config.EventCallback(this->config.callbackContext, Event::TransferComplete);
			}
		}
		LL_I2C_ClearFlag_STOP(this->instance);
	}
}