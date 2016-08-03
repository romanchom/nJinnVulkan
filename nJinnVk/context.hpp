#pragma once

#include <vulkan.hpp>
#include "Console.hpp"

namespace nJinn {
	extern class Context * context;

	class Context
	{
	private:
		enum {
			graphicsQueueIndex = 0,
			transferQueueIndex,
			computeQueueIndex,
			queueCount,
		};
		Console console;
		vk::Instance instance;
		vk::PhysicalDevice physicalDevice;
		vk::Device device;
		uint32_t queueFamilyIndicies[queueCount];
		vk::Queue queues[queueCount];

		uint32_t bufferMemoryTypeIndex;
		uint32_t uploadMemoryTypeIndex;
		bool isUploadMemoryCoherent;
		
		// debug section
		VkDebugReportCallbackEXT debugReportCallback;

		PFN_vkCreateDebugReportCallbackEXT CreateDebugReportCallback;
		PFN_vkDestroyDebugReportCallbackEXT	DestroyDebugReportCallback;
		const int validation;
		// end debug section
	
	public:
		Context();
		~Context();

		vk::Instance inst() { return instance; }
		vk::PhysicalDevice physDev() { return physicalDevice; }
		vk::Device dev() { return device; }

		vk::Queue mainQueue() { return queues[graphicsQueueIndex]; }
		vk::Queue transferQueue() { return queues[transferQueueIndex]; }
		vk::Queue computeQueue() { return queues[computeQueueIndex]; }

		uint32_t mainQueueFamilyIndex() { return queueFamilyIndicies[graphicsQueueIndex]; }
		uint32_t transferQueueFamilyIndex() { return queueFamilyIndicies[transferQueueIndex]; }
		uint32_t computeQueueFamilyIndex() { return queueFamilyIndicies[computeQueueIndex]; }

		uint32_t bufferMemoryType() { return bufferMemoryTypeIndex; }
		uint32_t uploadMemoryType() { return uploadMemoryTypeIndex; }
		bool isUploadMemoryTypeCoherent() { return isUploadMemoryCoherent; }
	};

}
