#pragma once

#include <vulkan.hpp>
#include "Console.hpp"

namespace nJinn {
	class Context
	{
	private:
		enum {
			graphicsQueueIndex = 0,
			transferQueueIndex,
			computeQueueIndex,
			queueCount,
		};
		static Context * context;
		~Context();
		Context();
		Console console;
		vk::Instance instance;
		vk::PhysicalDevice physicalDevice;
		vk::Device device;
		size_t queueFamilyIndicies[queueCount];
		vk::Queue queues[queueCount];

		uint32_t bufferMemoryTypeIndex;
		uint32_t uploadMemoryTypeIndex;
		bool isUploadMemoryCoherent;

		class PipelineFactory * pipelineFactory;

		// debug section
		vk::DebugReportCallbackEXT debugReportCallback;

		PFN_vkCreateDebugReportCallbackEXT CreateDebugReportCallback;
		PFN_vkDestroyDebugReportCallbackEXT	DestroyDebugReportCallback;
		bool validation;
		// end debug section
	
	public:
		static void create();
		static void destroy();

		static vk::Instance inst() { return context->instance; }
		static vk::PhysicalDevice physDev() { return context->physicalDevice; }
		static vk::Device dev() { return context->device; }

		static vk::Queue mainQueue() { return context->queues[graphicsQueueIndex]; }
		static vk::Queue transferQueue() { return context->queues[transferQueueIndex]; }
		static vk::Queue computeQueue() { return context->queues[computeQueueIndex]; }

		static size_t mainQueueFamilyIndex() { return context->queueFamilyIndicies[graphicsQueueIndex]; }
		static size_t transferQueueFamilyIndex() { return context->queueFamilyIndicies[transferQueueIndex]; }
		static size_t computeQueueFamilyIndex() { return context->queueFamilyIndicies[computeQueueIndex]; }

		static uint32_t bufferMemoryType() { return context->bufferMemoryTypeIndex; }
		static uint32_t uploadMemoryType() { return context->uploadMemoryTypeIndex; }
		static bool isUploadMemoryTypeCoherent() { return context->isUploadMemoryCoherent; }

		static class PipelineFactory & pipeFact() { return *context->pipelineFactory; }
	};
}
