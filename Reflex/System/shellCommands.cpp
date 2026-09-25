/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright (c) 2026 NotBlackMagic (PlumaLabs)
 *
 * File:    Instinct/Modules/System/shellCommands.cpp
 */

#include <cstdlib>

#include "shellModules.hpp"

extern char _stext[], _etext[];
extern char _sdata[], _edata[];
extern char _sbss[],  _ebss[];
extern char _estack[];

// Basic commands

static bool CommandHelp(const char* args) {
	(void)args;
	Logger::Instance().Write("--- Available Commands ---\r\n");
	// Iterate through the list of commands regsitered
	const Shell::CommandList* currentList = Shell::GetRegistry();
	while(currentList != nullptr) {
		// Iterate through commands in each list (module)
		const Shell::CommandEntry* entry = currentList->commandGroup;

		while(entry->name != nullptr) {
			char buffer[80];
			snprintf(buffer, sizeof(buffer), "  %-12s - %s\r\n", entry->name, (entry->helpText) ? entry->helpText : "");

			Logger::Instance().Write(buffer);
			entry++;	// Go to next command in this list/array
		}

		currentList = currentList->next;
	}
	Logger::Instance().Write("--------------------------\r\n");
	return true;
}

static bool CommandClear(const char* args) {
	(void)args;
	// ANSI Code to clear screen and move cursor home
	Logger::Instance().Write("\033[2J\033[H");
	return true;
}

// System status commands
static bool CommandInfo(const char* args) {
	(void)args;
	Logger::Instance().Write("\r\n--- SYSTEM INFORMATION ---\r\n");

	// Hardware information (from Board/BoardInfo.hpp)
	Logger::Instance().Write("[HARDWARE]\r\n");
	Logger::Instance().Printf("  Board:       %s (%s)\r\n", BoardInfo::Name, BoardInfo::Revision);
	Logger::Instance().Printf("  Designed:    %s in %s\r\n", BoardInfo::DesignDate, BoardInfo::DesignLocation);
	Logger::Instance().Printf("  Designer:    %s\r\n", BoardInfo::Designer);

	// Firmware information (From Config/Version.hpp)
	Logger::Instance().Write("[FIRMWARE]\r\n");
	Logger::Instance().Printf("  App Name:    %s\r\n", FW_NAME);
	Logger::Instance().Printf("  Version:     v%s\r\n", FW_VERSION_STR);
	Logger::Instance().Printf("  Build:       %s at %s (%s)\r\n", FW_BUILD_DATE, FW_BUILD_TIME, FW_BUILD_TYPE);
	Logger::Instance().Printf("  Commit:      %s\r\n", FW_GIT_HASH);
	Logger::Instance().Printf("  Compiler:    %s\r\n", FW_COMPILER);

	// Silicon and runtime information
	uint32_t revID = 0;	//*(uint32_t *)(REVID_BASE);
	uint32_t uid0 = READ_REG(*((uint32_t *)UID_BASE));
	uint32_t uid1 = READ_REG(*((uint32_t *)UID_BASE + 4U));
	uint32_t uid2 = READ_REG(*((uint32_t *)UID_BASE + 8U));

	Logger::Instance().Write("[SILICON]\r\n");
	Logger::Instance().Printf("  Device:      STM32C092 (Rev: 0x%04X)\r\n", revID);
	Logger::Instance().Printf("  UID:         %08lX-%08lX-%08lX\r\n", uid0, uid1, uid2);
	Logger::Instance().Printf("  SysClock:    %lu MHz\r\n", SystemCoreClock / 1000000);

	Logger::Instance().Write("--------------------------\r\n");
	return true;
}

static bool CommandVersion(const char* args) {
	(void)args;
	Logger::Instance().Printf("%s v%s\r\n", FW_NAME, FW_VERSION_STR);
	return true;
}

static bool CommandStatus(const char* args) {
	(void)args;
	Logger::Instance().Printf("CPU: STM32C092 @ 48MHz\r\n");
	Logger::Instance().Printf("Tick: %lu ms\r\n", Time::GetMs());
	// TBD: add Battery Voltage, Stack usage, etc.
	return true;
}

static bool CommandMemory(const char* args) {
	(void)args;

	// Calculate Usage
	uint32_t romUsed = (uint32_t)(_etext - _stext) + (uint32_t)(_edata - _sdata);
	uint32_t ramUsed = (uint32_t)(_edata - _sdata) + (uint32_t)(_ebss - _sbss);

	// Dynamic memory stuff
	uint32_t stackTotal = (uint32_t)_estack - (uint32_t)_ebss;
	uint32_t stackUsed  = (uint32_t)_estack - __get_MSP();

	// Calculate Percentages
	int romPct = (romUsed * 100) / BoardInfo::SizeROM;
	int ramPct = (ramUsed * 100) / BoardInfo::SizeRAM;
	int stackPct = (stackTotal > 0) ? (stackUsed * 100) / stackTotal : 0;

	// Print
	Logger::Instance().Write("\r\n--- MEMORY USAGE ---\r\n");
	// Internal Memory
	Logger::Instance().Printf("ROM    : %7lu / %8lu B (%d%%)\r\n", romUsed, BoardInfo::SizeROM, romPct);
	Logger::Instance().Printf("RAM    : %7lu / %8lu B (%d%%)\r\n", ramUsed, BoardInfo::SizeRAM, ramPct);
	// Dynamic Memory
	Logger::Instance().Printf("STACK  : %8lu / %8lu B (%d%%)\r\n", stackUsed, stackTotal, stackPct);
	Logger::Instance().Write("--------------------\r\n");

	return true;
}

// Control commands
static bool CommandReboot(const char* args) {
	(void)args;
	Logger::Instance().Write("Rebooting...\r\n");
	// NVIC_SystemReset();
	return true;
}

static bool CommandLog(const char* args) {
	int level = atoi(args);
	if (level >= 0 && level <= 6) {
		Logger::Instance().SetConsoleLevel((Logger::LogLevel)level);
		Logger::Instance().Printf("Log Level set to %d\r\n", level);
	}
	else {
		Logger::Instance().Write("Usage: log <0-6>\r\n");
	}
	return true;
}

// SYSTEM COMMANDS
static const Shell::CommandEntry systemCommands[] {
	// Basic commands
	{ "help",	CommandHelp,	"Lists commands" },
	{ "?",	CommandHelp,	"Lists commands" },
	{ "clear",CommandClear,	"Clear terminal" },

	// System status commands
	{"info",		CommandInfo, "Board & FW info" },
	{ "version",	CommandVersion,"Firmware info" },
	{ "status",	CommandStatus,	"System stats" },
	{ "mem",		CommandMemory,	"Memory usage" },

	// Control commands
	{ "reboot",	CommandReboot,		"Reboots system" },
	{ "log",		CommandLog,	"Set Log Level (0-6)" },
	
	{ nullptr,	nullptr,		nullptr } // Terminator
};

// Static memory for the node
static Shell::CommandList systemShellNode;

void RegisterSystemCommands() {
    Shell::RegisterCommands(&systemShellNode, systemCommands); //Register the table defined below
}