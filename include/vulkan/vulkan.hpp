#pragma once

#ifdef _WIN32
#define VK_USE_PLATFORM_WIN32_KHR 1
#endif

#include <cassert>
#define VKCPP_ENHANCED_MODE
#include "vk_cpp.h"


#ifdef _DEBUG
#include <iostream>

#define dc(val) \
	{\
		vk::Result res = val; \
		if(res != vk::Result::eVkSuccess) { \
			std::stringstream ss;\
			ss << "Vulkan error: " << (int) res << ", file: " __FILE__ ", line: " << __LINE__;\
			std::cout << ss.str();\
			throw std::runtime_error(ss.str());\
		}\
	}\

#else
#define dc(val) val
#endif