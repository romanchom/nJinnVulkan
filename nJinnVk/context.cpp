#include "stdafx.hpp"
#include "Context.hpp"

#include <iostream>
#include <iomanip>

#include "Config.hpp"

#include "ResourceUploader.hpp"
#include "Mesh.hpp"
#include "PipelineFactory.hpp"

namespace nJinn {
	static const char * appName = "nJinnVk";

	uint32_t getOptimalMemoryType(const vk::MemoryType * memoryTypes, size_t count, vk::MemoryPropertyFlags mandatoryFlags, vk::MemoryPropertyFlags optionalFlags) {
		uint32_t ret = -1;
		for (uint32_t i = 0; i < count; ++i) {
			vk::MemoryPropertyFlags flags = memoryTypes[i].propertyFlags();
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

	Context * Context::context = nullptr;

	Context::Context() :
		validation(Config::getValue<uint32_t>("debugLevel"))
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

		appInfo.apiVersion(VK_API_VERSION);
		appInfo.applicationVersion(1);
		appInfo.pApplicationName(appName);
		appInfo.pEngineName(appName);

		vk::InstanceCreateInfo instanceInfo;
		instanceInfo
			.ppEnabledExtensionNames(enabledExtensions.data())
			.enabledExtensionCount(enabledExtensions.size())
			.ppEnabledLayerNames(enabledLayers.data())
			.enabledLayerCount(enabledLayers.size())
			.pApplicationInfo(&appInfo);

		dc(vk::createInstance(&instanceInfo, nullptr, &instance));

		std::vector<vk::PhysicalDevice> physicalDevices;
		dc(vk::enumeratePhysicalDevices(instance, physicalDevices));

		physicalDevice = physicalDevices[0];

		std::vector<vk::QueueFamilyProperties> queuesProperties = vk::getPhysicalDeviceQueueFamilyProperties(physicalDevice);
		vk::QueueFlags queueTypes[3] = {
			vk::QueueFlagBits::eGraphics,
			vk::QueueFlagBits::eTransfer,
			vk::QueueFlagBits::eCompute,
		};

		struct queueLocation {
			size_t familyIndex = 0;
			size_t indexInFamily = 0;
		};

		queueLocation qLocations[queueCount];
		std::vector<size_t> queueInstanceCounts(queuesProperties.size());

		for (auto type : queueTypes) {
			for (size_t i = 0; i < queuesProperties.size(); ++i) {
				if (queuesProperties[i].queueFlags() & type) {
					qLocations[type].familyIndex = i;
					qLocations[type].indexInFamily = queueInstanceCounts[i]++;
					break;
				}
			}
		}

		float priorities[3] = { 1.0f, 1.0f, 1.0f };
		std::vector<vk::DeviceQueueCreateInfo> queueInfos;
		size_t queueIndex = 0;
		for (size_t i = 0; i < queuesProperties.size(); ++i) {
			if (queueInstanceCounts[i] > 0) {
				queueInfos.emplace_back();
				queueInfos.back()
					.queueFamilyIndex(i)
					.queueCount(queueInstanceCounts[i])
					.pQueuePriorities(priorities + queueIndex);
				queueIndex += queueInstanceCounts[i];
			}
		}

		enabledExtensions.clear();
		enabledExtensions.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);

		vk::PhysicalDeviceFeatures enabledFeatures;

		vk::getPhysicalDeviceFeatures(physicalDevice, &enabledFeatures);

		vk::DeviceCreateInfo deviceInfo;
		deviceInfo
			.queueCreateInfoCount(queueInfos.size())
			.pQueueCreateInfos(queueInfos.data())
			.pEnabledFeatures(&enabledFeatures)
			.ppEnabledExtensionNames(enabledExtensions.data())
			.enabledExtensionCount(enabledExtensions.size())
			.ppEnabledLayerNames(enabledLayers.data())
			.enabledLayerCount(enabledLayers.size());

		dc(vk::createDevice(physicalDevice, &deviceInfo, nullptr, &device));

		vk::PhysicalDeviceMemoryProperties memoryProperties;

		vk::getPhysicalDeviceMemoryProperties(physicalDevice, memoryProperties);

		for (size_t i = 0; i < queueCount; ++i) {
			auto & qLoc = qLocations[i];
			queueFamilyIndicies[i] = qLoc.familyIndex;
			vk::getDeviceQueue(device, qLoc.familyIndex, qLoc.indexInFamily, queues[i]);
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

		vk::PhysicalDeviceMemoryProperties memProps;
		vk::getPhysicalDeviceMemoryProperties(physicalDevice, memProps);
		
		bufferMemoryTypeIndex = getOptimalMemoryType(memProps.memoryTypes(), memProps.memoryTypeCount(),
			vk::MemoryPropertyFlagBits::eDeviceLocal,
			vk::MemoryPropertyFlags());

		uploadMemoryTypeIndex = getOptimalMemoryType(memProps.memoryTypes(), memProps.memoryTypeCount(),
			vk::MemoryPropertyFlagBits::eHostVisible,
			vk::MemoryPropertyFlagBits::eHostCoherent | 
			vk::MemoryPropertyFlagBits::eHostCached);

		isUploadMemoryCoherent = (memProps.memoryTypes()[uploadMemoryTypeIndex].propertyFlags() & vk::MemoryPropertyFlagBits::eHostCoherent);
	}

	void Context::create()
	{
		context = new Context();
		ResourceUploader::create();
		context->pipelineFactory = new PipelineFactory();
	}

	void Context::destroy()
	{
		vk::deviceWaitIdle(Context::dev());
		delete context->pipelineFactory;
		Mesh::collect();
		Shader::collect();
		ResourceUploader::destroy();
		delete context;
		context = nullptr;
	}

	Context::~Context()
	{
		if (validation) {
			DestroyDebugReportCallback(instance, debugReportCallback, nullptr);
		}
		vk::destroyDevice(device, nullptr);
		exit(0); // TODO FIXME temporary workaround crash during instance destruction
		vk::destroyInstance(instance, nullptr);
	}

	// FIXME
	static const int32_t ignoredCodes[] = {
		50, // Attempt to reset command buffer which is in use
		49, // Attempt to simultaneously execute command buffer without flag set
		// reusing command buffers reports errors, even if they have finished executing long time ago
	};

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
		VkDebugReportObjectTypeEXT objType,
		uint64_t srcObject,
		size_t location,
		int32_t msgCode,
		const char* pLayerPrefix,
		const char* pMsg,
		void* pUserData)
	{
		for (const int32_t code : ignoredCodes) {
			if (code == msgCode) return 0;
		}

		const char * type = nullptr;
		if (flags & VK_DEBUG_REPORT_ERROR_BIT_EXT) type = "ERR";
		else if (flags & VK_DEBUG_REPORT_WARNING_BIT_EXT) type = "WAR";
		else if (flags & VK_DEBUG_REPORT_INFORMATION_BIT_EXT) type = "INFO";
		else if (flags & VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT) type = "PERF";
		else if (flags & VK_DEBUG_REPORT_DEBUG_BIT_EXT) type = "DBG";
		
		std::cout << type;
		std::cout << " : object " << objectNames[objType] << "[" << std::hex << srcObject << std::dec;
		std::cout << "] : Layer " << pLayerPrefix;
		std::cout << " : Code " << msgCode << "\n";
		std::cout << "\"" << pMsg << "\"" << std::endl;

#ifdef _DEBUG
		DebugBreak();
#endif
		return 0;
	}
}