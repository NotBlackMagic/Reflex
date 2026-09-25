/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright (c) 2026 NotBlackMagic (PlumaLabs)
 *
 * File:    Instinct/Modules/System/console.cpp
 */

#include "console.hpp"

UART* Console::consolePort = nullptr;
Shell Console::shell;

void Console::Init(UART* uart) {
	consolePort = uart;

	Console::shell.Init();

	// Register all active command modules
	RegisterSystemCommands();
	RegisterI2CCommands();
	RegisterGPIOCommands();
	RegisterFOCCommands();
	RegisterGimbalCommands();
}

void Console::Run(uint32_t input) {
	(void)input; // Prevent unused parameter warning

	if(consolePort == nullptr) {
		return;
	}

	uint8_t rxBuffer[16];

	uint16_t len = consolePort->Receive(rxBuffer, sizeof(rxBuffer));
	if(len > 0) {
		shell.Input(rxBuffer, len);
	}
}