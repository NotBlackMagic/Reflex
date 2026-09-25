/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright (c) 2026 NotBlackMagic (PlumaLabs)
 *
 * File:    Instinct/Config/version.hpp.in
 * Author:  NotBlackMagic
 * Brief:   File contains dynamic firmware version information, filled by CMake.
 */

#pragma once

// Firmware version information, is completed by CMake

// Version Numbers
#define FW_VERSION_MAJOR    
#define FW_VERSION_MINOR    
#define FW_VERSION_PATCH    
#define FW_VERSION_STR      "0.1.0"

// Build Metadata
#define FW_NAME             "Instinct"
#define FW_BUILD_DATE       "2026-08-27"
#define FW_BUILD_TIME       "08:07:01"
#define FW_GIT_HASH         "a2073ec"
#define FW_BUILD_TYPE       "Debug"

// Compiler Info
#define FW_COMPILER         "GNU 14.3.1"
