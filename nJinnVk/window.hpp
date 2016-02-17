#pragma once

#include "application.hpp"
#include <cstdint>

namespace nJinn {
	class window {
	public:
		window(uint32_t width, uint32_t height);
		~window();
		static window * main() { return sMain; }
		bool run();
	private:
		static window * sMain;
		static void allocateConsole();

		void * mWindowHandle;
	};
}