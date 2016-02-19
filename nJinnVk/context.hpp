#pragma once

#include <vulkan.hpp>
#include "Console.hpp"

namespace nJinn {
	class Context
	{
		enum {
			graphicsQueueIndex = 0,
			transferQueueIndex,
			computeQueueIndex,
			queueCount,
		};
	public:
		static void create();
		static void destroy();
		static vk::Instance inst() { return context->instance; }
		static vk::PhysicalDevice physDev() { return context->physicalDevice; }
		static vk::Device dev() { return context->device; }
		static vk::Queue mainQueue() { return context->queues[graphicsQueueIndex]; }
		static vk::Queue transferQueue() { return context->queues[transferQueueIndex]; }
		static vk::Queue computeQueue() { return context->queues[computeQueueIndex]; }
	private:
		static Context * context;
		~Context();
		Context();
		Console console;
		vk::Instance instance;
		vk::PhysicalDevice physicalDevice;
		vk::Device device;
		size_t queueFamilyIndicies[queueCount];
		vk::Queue queues[queueCount];
	};
}
