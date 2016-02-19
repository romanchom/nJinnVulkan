#include "stdafx.hpp"
#include "Context.hpp"

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

	Context * Context::context = nullptr;

	Context::Context() :
		instance(nullptr),
		physicalDevice(nullptr),
		device(nullptr)

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
			vk::getDeviceQueue(device, qLoc.familyIndex, qLoc.indexInFamily, queues[i]);
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
		vk::destroyDevice(device, nullptr);
		vk::destroyInstance(instance, nullptr);
	}
}