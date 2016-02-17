#include "stdafx.hpp"
#include "context.hpp"

namespace nJinn {
	context * ctx;

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

	context::context()
	{
		vk::ApplicationInfo appInfo;

		appInfo.apiVersion(VK_API_VERSION);
		appInfo.applicationVersion(1);
		appInfo.pApplicationName(appName);
		appInfo.pEngineName(appName);

		vk::InstanceCreateInfo createInfo;
		createInfo.ppEnabledExtensionNames(extensions);
		createInfo.enabledExtensionCount(countof(extensions));
		createInfo.ppEnabledLayerNames(layers);
		createInfo.enabledLayerCount(countof(layers));
		createInfo.pApplicationInfo(&appInfo);

		dc(vk::createInstance(&createInfo, nullptr, &instance));
	}
	context::~context()
	{
		vk::destroyInstance(instance, nullptr);
	}

	void context::allocateConsole()
	{
		AllocConsole();
		AttachConsole(GetCurrentProcessId());
		freopen("CON", "w", stdout);
		SetConsoleTitle(TEXT("Debug console"));
	}
}