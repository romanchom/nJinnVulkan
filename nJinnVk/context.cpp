#include "stdafx.hpp"
#include "Context.hpp"

#include "Debug.hpp"

#include "Config.hpp"

#include "ResourceUploader.hpp"
#include "Mesh.hpp"
#include "PipelineFactory.hpp"
#include "UniformBuffer.hpp"

namespace nJinn {
	static const char * appName = "nJinnVk";

	uint32_t getOptimalMemoryType(const vk::MemoryType * memoryTypes, size_t count, vk::MemoryPropertyFlags mandatoryFlags, vk::MemoryPropertyFlags optionalFlags) {
		uint32_t ret = -1;
		for (uint32_t i = 0; i < count; ++i) {
			vk::MemoryPropertyFlags flags = memoryTypes[i].propertyFlags;
			if ((flags & mandatoryFlags) == mandatoryFlags) {
				if(ret == -1) ret = i;
				else if (flags & optionalFlags) ret = i;
			}
		}
		return ret;
	}

	VkBool32 VKAPI_PTR messageCallback(
		VkDebugReportFlagsEXT flags,
		VkDebugReportObjectTypeEXT objType,
		uint64_t srcObject,
		size_t location,
		int32_t msgCode,
		const char* pLayerPrefix,
		const char* pMsg,
		void* pUserData);

	Context * context = nullptr;

	Context::Context() :
		bufferMemoryTypeIndex(-1),
		uploadMemoryTypeIndex(-1),
		isUploadMemoryCoherent(false),
		debugReportCallback(nullptr),
		CreateDebugReportCallback(nullptr),
		DestroyDebugReportCallback(nullptr),
		validation(config.getValue<uint32_t>("debugVK"))
	{
		std::vector<const char *> enabledLayers;
		std::vector<const char *> enabledExtensions;

		enabledExtensions.push_back(VK_KHR_SURFACE_EXTENSION_NAME);
		enabledExtensions.push_back(VK_KHR_WIN32_SURFACE_EXTENSION_NAME);

		if (validation > 0) {
			enabledExtensions.push_back(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);
			enabledLayers.push_back("VK_LAYER_LUNARG_standard_validation");
		}

		vk::ApplicationInfo appInfo;

		appInfo
			.setApiVersion(0) // TODO possibly change 0 to something else
			.setApplicationVersion(1)
			.setPApplicationName(appName)
			.setPEngineName(appName);

		vk::InstanceCreateInfo instanceInfo;
		instanceInfo
			.setPpEnabledExtensionNames(enabledExtensions.data())
			.setEnabledExtensionCount((uint32_t) enabledExtensions.size())
			.setPpEnabledLayerNames(enabledLayers.data())
			.setEnabledLayerCount((uint32_t) enabledLayers.size())
			.setPApplicationInfo(&appInfo);

		instance = vk::createInstance(instanceInfo);

		std::vector<vk::PhysicalDevice> physicalDevices = instance.enumeratePhysicalDevices();

		physicalDevice = physicalDevices[0];

		std::vector<vk::QueueFamilyProperties> queuesProperties = physicalDevice.getQueueFamilyProperties();
		vk::QueueFlags queueTypes[3] = {
			vk::QueueFlagBits::eGraphics,
			vk::QueueFlagBits::eTransfer,
			vk::QueueFlagBits::eCompute,
		};

		struct queueLocation {
			uint32_t familyIndex = 0;
			uint32_t indexInFamily = 0;
			uint32_t fit = ~0u;
		};

		queueLocation qLocations[queueCount];
		std::vector<int> queueInstanceCounts(queuesProperties.size());

		for (int i = 0; i < queueCount; ++i) {
			auto type = queueTypes[i];
			for (uint32_t j = 0; j < queuesProperties.size(); ++j) {
				auto hasFlag = static_cast<bool>(queuesProperties[j].queueFlags & type);
				if (!hasFlag) continue;

				auto flags = static_cast<uint32_t>(queuesProperties[j].queueFlags);
				auto fit = __popcnt(flags);
				// if new queue type is a tighter fit
				if (fit < qLocations[i].fit) {
					qLocations[i].familyIndex = j;
					qLocations[i].fit = fit;
				}
			}
		}

		for (int i = 0; i < queueCount; ++i) {
			auto qIndex = qLocations[i].familyIndex;
			qLocations[i].indexInFamily = queueInstanceCounts[qIndex]++;
		}

		float priorities[3] = { 1.0f, 1.0f, 1.0f };
		std::vector<vk::DeviceQueueCreateInfo> queueInfos;
		int queueIndex = 0;
		for (int i = 0; i < queuesProperties.size(); ++i) {
			if (queueInstanceCounts[i] > 0) {
				queueInfos.emplace_back();
				queueInfos.back()
					.setQueueFamilyIndex(i)
					.setQueueCount(queueInstanceCounts[i])
					.setPQueuePriorities(priorities + queueIndex);
				queueIndex += queueInstanceCounts[i];
			}
		}

		enabledExtensions.clear();
		enabledExtensions.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);

		vk::PhysicalDeviceFeatures enabledFeatures = physicalDevice.getFeatures();

		vk::DeviceCreateInfo deviceInfo;
		deviceInfo
			.setQueueCreateInfoCount((uint32_t) queueInfos.size())
			.setPQueueCreateInfos(queueInfos.data())
			.setPEnabledFeatures(&enabledFeatures)
			.setPpEnabledExtensionNames(enabledExtensions.data())
			.setEnabledExtensionCount((uint32_t) enabledExtensions.size())
			.setPpEnabledLayerNames(enabledLayers.data())
			.setEnabledLayerCount((uint32_t) enabledLayers.size());

		device = physicalDevice.createDevice(deviceInfo);

		vk::PhysicalDeviceMemoryProperties memoryProperties = physicalDevice.getMemoryProperties();

		for (size_t i = 0; i < queueCount; ++i) {
			auto & qLoc = qLocations[i];
			queueFamilyIndicies[i] = qLoc.familyIndex;
			queues[i] = device.getQueue(qLoc.familyIndex, qLoc.indexInFamily);
		}

		const uint32_t levels[] = {
			0,													// 0 disabled
			VK_DEBUG_REPORT_ERROR_BIT_EXT,						// 1 error
			VK_DEBUG_REPORT_WARNING_BIT_EXT,					// 2 warning
			VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT,		// 3 perfwarn
			VK_DEBUG_REPORT_INFORMATION_BIT_EXT,				// 4 info
			VK_DEBUG_REPORT_DEBUG_BIT_EXT,						// 5 debug
		};

		if (validation) {
			CreateDebugReportCallback = (PFN_vkCreateDebugReportCallbackEXT) vkGetInstanceProcAddr(instance, "vkCreateDebugReportCallbackEXT");
			DestroyDebugReportCallback = (PFN_vkDestroyDebugReportCallbackEXT) vkGetInstanceProcAddr(instance, "vkDestroyDebugReportCallbackEXT");
			
			VkDebugReportCallbackCreateInfoEXT debugInfo;
			debugInfo.sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CREATE_INFO_EXT;
			debugInfo.pNext = NULL;
			debugInfo.pfnCallback = (PFN_vkDebugReportCallbackEXT)messageCallback;
			debugInfo.pUserData = NULL;
			debugInfo.flags = 0;
			for (int i = 1; i <= validation; ++i) {
				debugInfo.flags |= levels[i];
			}

			CreateDebugReportCallback(instance, &debugInfo, nullptr, &debugReportCallback);
		}

		bufferMemoryTypeIndex = getOptimalMemoryType(memoryProperties.memoryTypes, memoryProperties.memoryTypeCount,
			vk::MemoryPropertyFlagBits::eDeviceLocal,
			{});

		uploadMemoryTypeIndex = getOptimalMemoryType(memoryProperties.memoryTypes, memoryProperties.memoryTypeCount,
			vk::MemoryPropertyFlagBits::eHostVisible,
			vk::MemoryPropertyFlagBits::eDeviceLocal);

		isUploadMemoryCoherent = static_cast<bool>(memoryProperties.memoryTypes[uploadMemoryTypeIndex].propertyFlags & vk::MemoryPropertyFlagBits::eHostCoherent);

		physicalDevice.getProperties(&physicalDeviceProperties);

		mAligner = Aligner<vk::DeviceSize>(physicalDeviceProperties.limits.minUniformBufferOffsetAlignment);
	}

	Context::~Context()
	{
		device.destroy();
		if (debugReportCallback) {
			DestroyDebugReportCallback(instance, debugReportCallback, nullptr);
		}
		instance.destroy();
	}

	static const char * objectNames[] = {
		"Unknown",
		"Instance",
		"Physical device",
		"Device",
		"Queue",
		"Semaphore",
		"Command buffer",
		"Fence",
		"Device memory",
		"Buffer",
		"Image",
		"Event",
		"Query pool",
		"Buffer view",
		"Image view",
		"Shader module",
		"Pipeline cache",
		"Pipeline layout",
		"Render pass",
		"Pipeline",
		"Descriptor set layout",
		"Sampler",
		"Descriptor pool",
		"Descriptor set",
		"Framebuffer",
		"Command pool",
		"Surface khr",
		"Swapchain khr",
		"Debug report",
	};

	VkBool32 VKAPI_PTR messageCallback(
		VkDebugReportFlagsEXT flags,
		VkDebugReportObjectTypeEXT objectType,
		uint64_t object,
		size_t location,
		int32_t messageCode,
		const char* pLayerPrefix,
		const char* pMessage,
		void* pUserData)
	{
		const char * type = nullptr;
		if (flags & VK_DEBUG_REPORT_ERROR_BIT_EXT) type = "ERR";
		else if (flags & VK_DEBUG_REPORT_WARNING_BIT_EXT) type = "WAR";
		else if (flags & VK_DEBUG_REPORT_INFORMATION_BIT_EXT) type = "INFO";
		else if (flags & VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT) type = "PERF";
		else if (flags & VK_DEBUG_REPORT_DEBUG_BIT_EXT) type = "DBG";
		
		debug->log(type);
		debug->log(" : object ", objectNames[objectType], "[", std::hex, object, std::dec);
		debug->log("] : Layer ", pLayerPrefix);
		debug->log(" : Code ", messageCode, "\n");
		debug->log("\"", pMessage, "\"", "\n");

		if (flags & (VK_DEBUG_REPORT_ERROR_BIT_EXT | VK_DEBUG_REPORT_WARNING_BIT_EXT | VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT)) {
			DebugBreak();
		}

		return 0;
	}
}