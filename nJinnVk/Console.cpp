#include "stdafx.hpp"
#include "Console.hpp"

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <cstdio>

namespace nJinn {
	Console::Console()
	{
		AllocConsole();
		AttachConsole(GetCurrentProcessId());
		freopen("CON", "w", stdout);
		SetConsoleTitle(TEXT("Debug console"));
	}

	Console::~Console() {
		FreeConsole();
	}
}
