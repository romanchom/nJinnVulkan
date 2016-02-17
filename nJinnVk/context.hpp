#pragma once

#include <vulkan/vulkan.hpp>

namespace nJinn {
	class context
	{
	public:
		vk::Instance instance;
		
		context();
		~context();

	private:
		static void allocateConsole();
	};

	extern context * ctx;
}
