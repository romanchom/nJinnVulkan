#include "stdafx.hpp"
#include "Context.hpp"

#include "Config.hpp"
#include <iostream>
#include <iomanip>


namespace nJinn {
	static const char * appName = "nJinnVk";

	static const char * layers[] =
	{
		"VK_LAYER_LUNARG_threading",
		"VK_LAYER_LUNARG_mem_tracker",
		"VK_LAYER_LUNARG_object_tracker",
		"VK_LAYER_LUNARG_draw_state",
		"VK_LAYER_LUNARG_param_checker",
		"VK_LAYER_LUNARG_swapchain",
		"VK_LAYER_LUNARG_device_limits",
		"VK_LAYER_LUNARG_image",
		"VK_LAYER_GOOGLE_unique_objects",
	};

	static const char * extensions[] =
	{
		VK_KHR_SURFACE_EXTENSION_NAME,
		VK_KHR_WIN32_SURFACE_EXTENSION_NAME,
		VK_EXT_DEBUG_REPORT_EXTENSION_NAME,
	};

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
		instance(nullptr),
		physicalDevice(nullptr),
		device(nullptr),
		validation(false)
	{
		vk::ApplicationInfo appInfo;

		appInfo.apiVersion(VK_API_VERSION);
		appInfo.applicationVersion(1);
		appInfo.pApplicationName(appName);
		appInfo.pEngineName(appName);

		vk::InstanceCreateInfo instanceInfo;
		instanceInfo.ppEnabledExtensionNames(extensions);
		instanceInfo.enabledExtensionCount(countof(extensions));
		instanceInfo.ppEnabledLayerNames(layers);
		instanceInfo.enabledLayerCount(countof(layers));
		instanceInfo.pApplicationInfo(&appInfo);

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

		const char * deviceExtensions[] = {
			VK_KHR_SWAPCHAIN_EXTENSION_NAME
		};

		vk::DeviceCreateInfo deviceInfo;
		deviceInfo
			.queueCreateInfoCount(queueInfos.size())
			.pQueueCreateInfos(queueInfos.data())
			.pEnabledFeatures(nullptr)
			.enabledExtensionCount(countof(deviceExtensions))
			.ppEnabledExtensionNames(deviceExtensions)
			.enabledLayerCount(countof(layers))
			.ppEnabledLayerNames(layers);

		dc(vk::createDevice(physicalDevice, &deviceInfo, nullptr, &device));

		vk::PhysicalDeviceMemoryProperties memoryProperties;

		vk::getPhysicalDeviceMemoryProperties(physicalDevice, memoryProperties);

		for (size_t i = 0; i < queueCount; ++i) {
			auto & qLoc = qLocations[i];
			queueFamilyIndicies[i] = qLoc.familyIndex;
			vk::getDeviceQueue(device, qLoc.familyIndex, qLoc.indexInFamily, queues[i]);
		}

		validation = Config::getValue<uint32_t>("debugLevel");
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
	}

	void Context::create()
	{
		context = new Context();
	}

	void Context::destroy()
	{
		delete context;
		context = nullptr;
	}

	Context::~Context()
	{
		vk::deviceWaitIdle(device);
		if (validation) {
			DestroyDebugReportCallback(instance, debugReportCallback, nullptr);
		}
		vk::destroyDevice(device, nullptr);
		vk::destroyInstance(instance, nullptr);
	}

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
		bool ignore = false;
		for (const int32_t code : ignoredCodes) {
			if (code == msgCode) {
				ignore = true;
				break;
			}
		}
		if (ignore) return 0;

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