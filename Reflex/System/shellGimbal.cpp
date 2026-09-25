/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright (c) 2026 NotBlackMagic (PlumaLabs)
 *
 * File:    Instinct/Modules/System/shellGimbal.cpp
 */

#include "shellModules.hpp"
#include "fusion.hpp"

static bool CommandGimbal(const char* args) {
	if(args == nullptr || args[0] == '\0') {
		Logger::Instance().Write("Usage: gimbal <angles|status>\r\n");
		return true;
	}

	if(strcmp(args, "angles") == 0) {
		int32_t pitchRaw = SensorFusion::GetPitch();
		int32_t rollRaw = SensorFusion::GetRoll();
		int32_t yawRaw = SensorFusion::GetYaw();

		// Format the fixed-point numbers
		int pInt = pitchRaw / 100;
		int pDec = pitchRaw < 0 ? -(pitchRaw % 100) : (pitchRaw % 100);

		int rInt = rollRaw / 100;
		int rDec = rollRaw < 0 ? -(rollRaw % 100) : (rollRaw % 100);

		int yInt = yawRaw / 100;
		int yDec = yawRaw < 0 ? -(yawRaw % 100) : (yawRaw % 100);

		Logger::Instance().Printf("Pitch: %d.%02d | Roll: %d.%02d | Yaw: %d.%02d\r\n", pInt, pDec, rInt, rDec, yInt, yDec);
	}
	else {
		Logger::Instance().Printf("Unknown gimbal command: '%s'\r\n", args);
	}

	return true;
}

static const Shell::CommandEntry gimbalCommands[] = {
	{ "gimbal", CommandGimbal, "Read gimbal telemetry and state" },
	{ nullptr, nullptr, nullptr }	// Terminator
};

static Shell::CommandList gimbalShellNode;

void RegisterGimbalCommands() {
	Shell::RegisterCommands(&gimbalShellNode, gimbalCommands);
}