/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright (c) 2026 NotBlackMagic (PlumaLabs)
 *
 * File:	Instinct/Modules/System/shellFOC.cpp
 */

#include "shellModules.hpp"

extern volatile bool pauseFOCUpdates;

// Helper to route the command to the correct axis
static TMC6460* GetDriver(const char* axisStr) {
	if(strcmp(axisStr, "pitch") == 0) {
		return &tmc6460X;
	}
	if(strcmp(axisStr, "roll") == 0) {
		return &tmc6460Y;
	}
	if(strcmp(axisStr, "yaw") == 0) {
		return &tmc6460Z;
	}
	return nullptr;
}

static bool CommandFOC(const char* args) {
	if(args == nullptr || args[0] == '\0') {
		Logger::Instance().Write(	"Usage: bldc <axis> <command> [args]\r\n"
										"Axes: pitch, roll, yaw\r\n"
										"Commands:\r\n"
										"  enable|disable|clear\r\n"
										"  status|sense|actual|align\r\n"
										"  pi <cur|vel|pos> <pGain> <iGain>\r\n"
										"  limit <cur|vel|pos> <limit>\r\n"
										"  mode\r\n"
										"  ramp <amax> <vmax>\r\n"
										"  vel <velocity>\r\n"
										"  pos <position>\r\n"
		);
		return true;
	}

	char axis[10];
	char action[16];

	// Parse the axis and the action/command
	int count = sscanf(args, "%9s %15s", axis, action);
	if(count < 2) {
		return false;
	}

	TMC6460* driver = GetDriver(axis);
	if(driver == nullptr) {
		Logger::Instance().Printf("Invalid axis: '%s'. Use pitch, roll, or yaw.\r\n", axis);
		return true;
	}

	// Skip the axis and action strings to parse arguments dynamically
	const char* subArgs = args + strlen(axis) + strlen(action) + 2;

	pauseFOCUpdates = true;
	__DMB();

	if(strcmp(action, "enable") == 0) {
		if(driver->EnableDriver() == Status::Ok) {
			Logger::Instance().Printf("FOC %s Enabled.\r\n", axis);
		}
		else {
			Logger::Instance().Printf("Failed to enable FOC %s.\r\n", axis);
		}
	}
	else if(strcmp(action, "disable") == 0) {
		driver->DisableDriver();
		Logger::Instance().Printf("FOC %s Disabled.\r\n", axis);
	}
	else if(strcmp(action, "clear") == 0) {
		if(driver->ClearFaults() == Status::Ok) {
			Logger::Instance().Printf("FOC %s Faults Cleared.\r\n", axis);
		}
		else {
			Logger::Instance().Printf("Failed to clear faults %s.\r\n", axis);
		}
	}
	else if(strcmp(action, "status") == 0) {
		uint32_t flags = 0;
		if(driver->GetSystemStatus(flags) == Status::Ok) {
			Logger::Instance().Printf("FOC %s Status Flags: 0x%08X\r\n", axis, flags);
			Logger::Instance().Printf("    GDRV_ON: %d\r\n", ((flags >> 31) & 0x01));
			Logger::Instance().Printf("    SYS_RDY: %d\r\n", ((flags >> 30) & 0x01));
			Logger::Instance().Printf("    OVR_TEMP: %d\r\n", ((flags >> 29) & 0x01));
			Logger::Instance().Printf("    OV_VM_LIM: %d\r\n", ((flags >> 28) & 0x01));
			Logger::Instance().Printf("    VM_TEMP_CLIP: %d\r\n", ((flags >> 27) & 0x01));
			Logger::Instance().Printf("    AIN_CLIP: %d\r\n", ((flags >> 26) & 0x01));
			Logger::Instance().Printf("    I_CLIP: %d\r\n", ((flags >> 25) & 0x01));
			Logger::Instance().Printf("    OVL_CUR: %d\r\n", ((flags >> 24) & 0x01));
			Logger::Instance().Printf("    HALL_FAIL: %d\r\n", ((flags >> 23) & 0x01));
			Logger::Instance().Printf("    ABN_FAIL: %d\r\n", ((flags >> 22) & 0x01));
			Logger::Instance().Printf("    REF_H: %d\r\n", ((flags >> 21) & 0x01));
			Logger::Instance().Printf("    REF_LR: %d\r\n", ((flags >> 20) & 0x01));
			Logger::Instance().Printf("    V_ZERO: %d\r\n", ((flags >> 19) & 0x01));
			Logger::Instance().Printf("    V_REACH: %d\r\n", ((flags >> 18) & 0x01));
			Logger::Instance().Printf("    TARG_REACH: %d\r\n", ((flags >> 17) & 0x01));
			Logger::Instance().Printf("    STALL_FAIL: %d\r\n", ((flags >> 16) & 0x01));
			Logger::Instance().Printf("    IO_CTRL: %d\r\n", ((flags >> 15) & 0x01));
			Logger::Instance().Printf("    DIFF_ENC: %d\r\n", ((flags >> 14) & 0x01));
			Logger::Instance().Printf("    OT_FAIL: %d\r\n", ((flags >> 13) & 0x01));
			Logger::Instance().Printf("    SHRT_FAIL: %d\r\n", ((flags >> 12) & 0x01));
			Logger::Instance().Printf("    VEL_FAIL: %d\r\n", ((flags >> 11) & 0x01));
			Logger::Instance().Printf("    TEMP_FAIL: %d\r\n", ((flags >> 10) & 0x01));
			Logger::Instance().Printf("    ADC_FAIL: %d\r\n", ((flags >> 9) & 0x01));
			Logger::Instance().Printf("    ERES_FAIL: %d\r\n", ((flags >> 8) & 0x01));
			Logger::Instance().Printf("    PLL_FAIL: %d\r\n", ((flags >> 7) & 0x01));
			Logger::Instance().Printf("    PW_FAIL: %d\r\n", ((flags >> 6) & 0x01));
			Logger::Instance().Printf("    UV_VM_FAIL: %d\r\n", ((flags >> 5) & 0x01));
			Logger::Instance().Printf("    CP_FAIL: %d\r\n", ((flags >> 4) & 0x01));
			Logger::Instance().Printf("    IOF_FAIL: %d\r\n", ((flags >> 3) & 0x01));
			Logger::Instance().Printf("    UART_FAIL: %d\r\n", ((flags >> 2) & 0x01));
			Logger::Instance().Printf("    SPI_FAIL: %d\r\n", ((flags >> 1) & 0x01));
			Logger::Instance().Printf("    PWRUP_FAIL: %d\r\n", ((flags >> 0) & 0x01));
		}
		else {
			Logger::Instance().Printf("Failed to read %s status.\r\n", axis);
		}
	}
	else if(strcmp(action, "sense") == 0) {
		int32_t cur1, cur2, cur3, tempInt, tempExt;
		Status status = driver->GetCurrent(cur1, cur2, cur3);
		if(status == Status::Ok) {
			status = driver->GetTemperature(tempInt, tempExt);
		}
		if(status == Status::Ok) {
			Logger::Instance().Printf("FOC %s Sense: \r\n", axis);
			Logger::Instance().Printf("    Current 1 (mA): %d\r\n", cur1);
			Logger::Instance().Printf("    Current 2 (mA): %d\r\n", cur2);
			Logger::Instance().Printf("    Current 3 (mA): %d\r\n", cur3);
			Logger::Instance().Printf("    Temperature Internal (deg): %d,%d\r\n", (tempInt / 100), (tempInt % 100));
			Logger::Instance().Printf("    Temperature External (mV): %d\r\n", tempExt);
		}
		else {
			Logger::Instance().Printf("Failed to read %s sense.\r\n", axis);
		}
	}
	else if(strcmp(action, "actual") == 0) {
		int32_t torque, flux, vel, pos;
		uint16_t phiE, phiConvA, phiLookA, phiExtA;
		Status status = driver->GetTorque(torque, flux);
		if(status == Status::Ok) {
			status = driver->GetVelocity(vel);
		}
		if(status == Status::Ok) {
			status = driver->GetPosition(pos);
		}
		if(status == Status::Ok) {
			status = driver->GetFeedback(phiE, phiConvA, phiLookA, phiExtA);
		}
		if(status == Status::Ok) {
			Logger::Instance().Printf("FOC %s ACTUAL: \r\n", axis);
			Logger::Instance().Printf("    Torque: %d\r\n", torque);
			Logger::Instance().Printf("    Flux: %d\r\n", flux);
			Logger::Instance().Printf("    Velocity: %d\r\n", vel);
			Logger::Instance().Printf("    Position: %d\r\n", pos);
			Logger::Instance().Printf("    PHI A: %d -> %d -> %d -> %d\r\n", phiExtA, phiConvA, phiLookA, phiE);
		}
		else {
			Logger::Instance().Printf("Failed to read %s actual.\r\n", axis);
		}
	}
	else if(strcmp(action, "align") == 0) {
		Logger::Instance().Printf("Aligning %s motor. Please wait 1 second...\r\n", axis);
		driver->RunEncoderAlignment(1000); // Blocks for 1 sec internally
		Logger::Instance().Printf("FOC %s: Commutation aligned.\r\n", axis);
	}
	else if(strcmp(action, "pi") == 0) {
		char type[16];
		int pGain = 0, iGain = 0;
		
		// Parse the sub-command and the two integers
		if(sscanf(subArgs, "%3s %d %d", type, &pGain, &iGain) == 3) {
			if(strcmp(type, "cur") == 0) {
				if(driver->SetCurrentPI(pGain, iGain) == Status::Ok) {
					Logger::Instance().Printf("FOC %s Current PI set to: P=%d, I=%d\r\n", axis, pGain, iGain);
				}
				else {
					Logger::Instance().Printf("Failed to set %s Current PI.\r\n", axis);
				}
			} 
			else if(strcmp(type, "vel") == 0) {
				if(driver->SetVelocityPI(pGain, iGain) == Status::Ok) {
					Logger::Instance().Printf("FOC %s Velocity PI set to: P=%d, I=%d\r\n", axis, pGain, iGain);
				}
				else {
					Logger::Instance().Printf("Failed to set %s Velocity PI.\r\n", axis);
				}
			} 
			else if(strcmp(type, "pos") == 0) {
				if(driver->SetPositionPI(pGain, iGain) == Status::Ok) {
					Logger::Instance().Printf("FOC %s Position PI set to: P=%d, I=%d\r\n", axis, pGain, iGain);
				}
				else {
					Logger::Instance().Printf("Failed to set %s Position PI.\r\n", axis);
				}
			} 
			else {
				Logger::Instance().Printf("Unknown PI loop: '%s'. Use cur, vel, or pos.\r\n", type);
			}
		}
		else {
			Logger::Instance().Write("Usage: foc <axis> pi <cur|vel|pos> <pGain> <iGain>\r\n");
		}
	}
	else if(strcmp(action, "limit") == 0) {
		char type[16];
		int limit;
		
		// Parse the sub-command and the single integers
		if(sscanf(subArgs, "%3s %d", type, &limit) == 2) {
			if(strcmp(type, "cur") == 0) {
				// driver->SetCurrentLimits(limit);
				if(driver->SetTorqueLimits(limit, limit) == Status::Ok) {
					Logger::Instance().Printf("FOC %s Current Limit set to: %d\r\n", axis, limit);
				}
				else {
					Logger::Instance().Printf("Failed to set %s Current Limit.\r\n", axis);
				}
			} 
			else if(strcmp(type, "vel") == 0) {
				if(driver->SetVelocityLimits(limit) == Status::Ok) {
					Logger::Instance().Printf("FOC %s Velocity Limit set to: %d\r\n", axis, limit);
				}
				else {
					Logger::Instance().Printf("Failed to set %s Velocity Limit.\r\n", axis);
				}
			} 
			else if(strcmp(type, "pos") == 0) {
				if(driver->SetPositionLimits(limit, limit) == Status::Ok) {
					Logger::Instance().Printf("FOC %s Position Limit set to: %d\r\n", axis, limit);
				}
				else {
					Logger::Instance().Printf("Failed to set %s Position Limit.\r\n", axis);
				}
			} 
			else {
				Logger::Instance().Printf("Unknown limit: '%s'. Use cur, vel, or pos.\r\n", type);
			}
		}
		else {
			Logger::Instance().Write("Usage: foc <axis> limit <cur|vel|pos> <limit>\r\n");
		}
	}
	else if(strcmp(action, "mode") == 0) {
		char mode[16];
		// Parse the mode
		if(sscanf(subArgs, "%5s", mode) == 1) {
			if(strcmp(mode, "flux") == 0) {
				driver->SetMotionMode(0x01, 0x02);
			}
			else if(strcmp(mode, "vel") == 0) {
				driver->SetMotionMode(0x01, 0x03);
			}
			else if(strcmp(mode, "pos") == 0) {
				driver->SetMotionMode(0x00, 0x04);
			}
			else {
				Logger::Instance().Printf("Unknown mode: '%s'. Use flux, vel, or pos.\r\n", mode);
			}
		}
	}
	else if(strcmp(action, "ramp") == 0) {
		int aMax = 0, vMax = 0;
		if(sscanf(subArgs, "%d %d", &aMax, &vMax) == 2) {
			if(driver->SetRampLimits(aMax, vMax) == Status::Ok) {
				Logger::Instance().Printf("FOC %s Ramp set: A=%d, V=%d\r\n", axis, aMax, vMax);
			}
			else {
				Logger::Instance().Printf("Failed to set %s FOC ramp.\r\n", axis);
			}
		}
		else {
			Logger::Instance().Write("Usage: foc <axis> ramp <aMax> <vMax>\r\n");
		}
	}
	else if(strcmp(action, "vel") == 0) {
		int vel = 0;
		if(sscanf(subArgs, "%d", &vel) == 1) {
			if(driver->SetVelocity(vel) == Status::Ok) {
				Logger::Instance().Printf("FOC %s Target Velocity set to: %d\r\n", axis, vel);
			}
			else {
				Logger::Instance().Printf("Failed to set %s target velocity.\r\n", axis);
			}
		}
		else {
			Logger::Instance().Write("Usage: foc <axis> vel <velocity>\r\n");
		}
	}
	else if(strcmp(action, "pos") == 0) {
		int pos = 0;
		if(sscanf(subArgs, "%d", &pos) == 1) {
			if(driver->SetPosition(pos) == Status::Ok) {
				Logger::Instance().Printf("FOC %s Target Position set to: %d\r\n", axis, pos);
			}
			else {
				Logger::Instance().Printf("Failed to set %s target position.\r\n", axis);
			}
		}
		else {
			Logger::Instance().Write("Usage: foc <axis> pos <position>\r\n");
		}
	}
	else {
		Logger::Instance().Printf("Unknown foc command: %s\r\n", action);
	}

	__DMB();
	pauseFOCUpdates = false;

	return true;
}

static const Shell::CommandEntry focCommands[] = {
	{ "foc", CommandFOC, "Control TMC6460 Motor Driver" },
	{ nullptr, nullptr, nullptr } // Terminator
};

static Shell::CommandList focShellNode;

void RegisterFOCCommands() {
	Shell::RegisterCommands(&focShellNode, focCommands);
}