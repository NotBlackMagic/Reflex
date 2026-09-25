#pragma once

#include <stdint.h>

#include "hardware.hpp"

#include "shell.hpp"
#include "shellModules.hpp"

class Console {
	public:
		static void Init(UART* uart);
		static void Run(uint32_t input = 0);

	private:
		static UART* consolePort;
		static Shell shell;
};