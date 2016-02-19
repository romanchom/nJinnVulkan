#include "stdafx.hpp"
#include "Screen.hpp"


#include <cstdio>
#include <algorithm>
#include "Application.hpp"
#include "Context.hpp"
#include "CommandBuffer.hpp"

namespace nJinn {
	LRESULT CALLBACK WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

	Screen::Screen(uint32_t width, uint32_t height) :
		mWindowHandle(nullptr),
		queueIndex(-1),
		swapChain(nullptr),
		renderPass(nullptr),
		surface(nullptr),
		width(-1),
		height(-1),
		currentFrameIndex(0),
		currentFrame(frames + currentFrameIndex),
		currentAcquireFrameSemaphore(nullptr),
		currentFence(0)
	{
		WNDCLASSEX windowClass = { 0 };
		windowClass.cbSize = sizeof(WNDCLASSEX);
		windowClass.style = CS_HREDRAW | CS_VREDRAW;
		windowClass.lpfnWndProc = WindowProc;
		windowClass.hInstance = Application::shInstance;
		windowClass.hCursor = LoadCursor(NULL, IDC_ARROW);
		windowClass.lpszClassName = L"WindowClass1";
		RegisterClassEx(&windowClass);

		RECT windowRect = { 0, 0, width, height };
		AdjustWindowRect(&windowRect, WS_OVERLAPPEDWINDOW, FALSE);

		// Create the Window and store a handle to it.
		mWindowHandle = CreateWindowEx(NULL,
			L"WindowClass1",
			L"nJinn",
			WS_OVERLAPPEDWINDOW,
			100,
			50,
			windowRect.right - windowRect.left,
			windowRect.bottom - windowRect.top,
			NULL,		// We have no parent Window, NULL.
			NULL,		// We aren't using menus, NULL.
			Application::shInstance,
			NULL);		// We aren't using multiple windows, NULL.

		ShowWindow((HWND) mWindowHandle, Application::snCmdShow);

		vk::Win32SurfaceCreateInfoKHR surfaceCreateInfo = {};
		surfaceCreateInfo.hinstance(Application::shInstance);
		surfaceCreateInfo.hwnd((HWND) mWindowHandle);
		dc(vk::createWin32SurfaceKHR(Context::inst(), &surfaceCreateInfo, nullptr, &surface));

		std::vector<vk::QueueFamilyProperties> queueProperties = vk::getPhysicalDeviceQueueFamilyProperties(Context::physDev());
		for (int i = 0; i < queueProperties.size(); ++i) {
			if (queueProperties[i].queueFlags() & vk::QueueFlagBits::eGraphics) {
				uint32_t supported = 0;
				dc(vk::getPhysicalDeviceSurfaceSupportKHR(Context::physDev(), i, surface, supported));
				if (supported) {
					queueIndex = i;
					break;
				}
			}
		}

		assert(queueIndex != -1);

		std::vector<vk::SurfaceFormatKHR> surfaceFormats;
		dc(vk::getPhysicalDeviceSurfaceFormatsKHR(Context::physDev(), surface, surfaceFormats));

		if (1 == surfaceFormats.size() && vk::Format::eUndefined == surfaceFormats[0].format()) {
			colorFormat = vk::Format::eB8G8R8A8Srgb;
		}
		else {
			vk::Format prefferedFormats[] = { 
				vk::Format::eB8G8R8A8Srgb,
				vk::Format::eR8G8B8A8Srgb,
				vk::Format::eA8B8G8R8SrgbPack32,
				vk::Format::eR8G8B8A8Unorm,
				vk::Format::eB8G8R8A8Unorm,
			};
			for (vk::Format f : prefferedFormats) {
				auto it = surfaceFormats.begin();
				while (surfaceFormats.end() != it) {
					if (it->format() == f) break;
					++it;
				}
				if (surfaceFormats.end() != it) {
					colorFormat = f;
					colorSpace = it->colorSpace();
					break;
				}
			}
		}

		resize(width, height);
	}

	Screen::~Screen()
	{
		vk::destroyRenderPass(Context::dev(), renderPass, nullptr);
		vk::destroySurfaceKHR(Context::inst(), surface, nullptr);
	}

	void Screen::resize(uint32_t width, uint32_t height)
	{
		vk::SwapchainKHR oldSwapChain = swapChain;

		vk::SurfaceCapabilitiesKHR surfaceCaps;
		dc(vk::getPhysicalDeviceSurfaceCapabilitiesKHR(Context::physDev(), surface, surfaceCaps));

		std::vector<vk::PresentModeKHR> presentModes;
		vk::getPhysicalDeviceSurfacePresentModesKHR(Context::physDev(), surface, presentModes);

		vk::Extent2D swapChainExtent;
		if (surfaceCaps.currentExtent().width() == -1) {
			swapChainExtent.width(width);
			swapChainExtent.height(height);
		} else {
			swapChainExtent = surfaceCaps.currentExtent();
		}
		this->width = swapChainExtent.width();
		this->height = swapChainExtent.height();

		vk::PresentModeKHR presentMode = vk::PresentModeKHR::eVkPresentModeFifoKhr;
		for (auto pm : presentModes) {
			if (pm == vk::PresentModeKHR::eVkPresentModeMailboxKhr) {
				presentMode = pm;
				break;
			}
		}

		vk::SwapchainCreateInfoKHR swapChainInfo;
		swapChainInfo
			.surface(surface)
			.minImageCount(3)
			.imageFormat(colorFormat)
			.imageColorSpace(colorSpace)
			.imageExtent(swapChainExtent)
			.imageUsage(vk::ImageUsageFlagBits::eColorAttachment)
			.preTransform(vk::SurfaceTransformFlagBitsKHR::eIdentity)
			.imageArrayLayers(1)
			.queueFamilyIndexCount(0)
			.imageSharingMode(vk::SharingMode::eExclusive)
			.pQueueFamilyIndices(nullptr)
			.presentMode(presentMode)
			.oldSwapchain(oldSwapChain)
			.clipped(true)
			.compositeAlpha(vk::CompositeAlphaFlagBitsKHR::eOpaque);

		dc(vk::createSwapchainKHR(Context::dev(), &swapChainInfo, nullptr, &swapChain));

		if (oldSwapChain) {
			vk::destroySwapchainKHR(Context::dev(), oldSwapChain, nullptr);
		}

		std::vector<vk::Image> images;
		dc(vk::getSwapchainImagesKHR(Context::dev(), swapChain, images));

		assert(3 == images.size());

		vk::AttachmentDescription attachment;
		attachment
			.format(colorFormat)
			.samples(vk::SampleCountFlagBits::e1)
			.loadOp(vk::AttachmentLoadOp::eClear)
			.storeOp(vk::AttachmentStoreOp::eStore)
			.stencilLoadOp(vk::AttachmentLoadOp::eDontCare)
			.stencilStoreOp(vk::AttachmentStoreOp::eDontCare)
			.initialLayout(vk::ImageLayout::eColorAttachmentOptimal)
			.finalLayout(vk::ImageLayout::eColorAttachmentOptimal);
			
		vk::AttachmentReference attachmentReference(0, vk::ImageLayout::eColorAttachmentOptimal);

		vk::SubpassDescription subpass;
		subpass
			.pipelineBindPoint(vk::PipelineBindPoint::eGraphics)
			.colorAttachmentCount(1)
			.pColorAttachments(&attachmentReference);


		vk::RenderPassCreateInfo renderPassInfo;
		renderPassInfo
			.attachmentCount(1)
			.pAttachments(&attachment)
			.subpassCount(1)
			.pSubpasses(&subpass);

		dc(vk::createRenderPass(Context::dev(), &renderPassInfo, nullptr, &renderPass));

		for (size_t i = 0; i < 3; ++i) {
			frames[i].create(*this, i, images[i]);

			// TODO transition into present layout VK_IMAGE_LAYOUT_PRESENT_SRC_KHR or something meaningfull
		}
		vk::FenceCreateInfo fenceInfo;
		fenceInfo.flags(vk::FenceCreateFlagBits::eSignaled);
		for (size_t i = 0; i < maxQueuedFrames; ++i) {
			dc(vk::createFence(Context::dev(), &fenceInfo, nullptr, fences + i));
		}

		acquireFrame();
		present();
	}

	void Screen::present()
	{
		std::cout << "Presenting frame: " << currentFrameIndex << std::endl;
		currentFrame->present();
	}

	void Screen::acquireFrame()
	{
		vk::waitForFences(Context::dev(), 1, &fences[currentFence], 1, -1);
		vk::resetFences(Context::dev(), 1, &fences[currentFence]);

		uint32_t imageIndex;
		currentAcquireFrameSemaphore = currentFrame->imageAquiredSemaphore;
		dc(vk::acquireNextImageKHR(Context::dev(), swapChain, -1, currentAcquireFrameSemaphore, fences[currentFence], imageIndex));
		currentFrameIndex = imageIndex;
		currentFrame = frames + currentFrameIndex;
		++currentFence %= maxQueuedFrames;
		std::cout << "Acquired frame: " << currentFrameIndex << std::endl;
	}

	void Screen::testBlink()
	{
	}

	bool Screen::shouldClose()
	{
		MSG msg = { 0 };
		while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
			TranslateMessage(&msg);
			DispatchMessage(&msg);
			if (msg.message == WM_QUIT) return true;
		}

		return false;
	}

	LRESULT CALLBACK WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
	{
		// Handle destroy/shutdown messages.
		switch (message)
		{
		case WM_DESTROY:
			PostQuitMessage(0);
			return 0;
		}

		// Handle any messages the switch statement didn't.
		return DefWindowProc(hWnd, message, wParam, lParam);
	}

	Screen::Frame::Frame() :
		image(nullptr),
		view(nullptr),
		frameBuffer(nullptr),
		imageAquiredSemaphore(nullptr),
		renderingCompleteSemaphore(nullptr)
	{}

	Screen::Frame::~Frame()
	{
		destroy();
	}

	void Screen::Frame::create(const Screen & info, size_t index, vk::Image img)
	{
		if (image) {
			destroy();
		}

		image = img;

		vk::ImageViewCreateInfo viewInfo;
		viewInfo
			.format(info.colorFormat)
			.components(vk::ComponentMapping())
			.subresourceRange(vk::ImageSubresourceRange(
				vk::ImageAspectFlagBits::eColor,
				0, 1, 0, 1))
			.viewType(vk::ImageViewType::e2D)
			.flags(0)
			.image(image);

		dc(vk::createImageView(Context::dev(), &viewInfo, nullptr, &view));

		vk::FramebufferCreateInfo framebufferInfo;
		framebufferInfo
			.attachmentCount(1)
			.pAttachments(&view)
			.renderPass(info.renderPass)
			.height(info.height)
			.width(info.width)
			.layers(1);

		vk::createFramebuffer(Context::dev(), &framebufferInfo, nullptr, &frameBuffer);

		vk::SemaphoreCreateInfo semaphoreInfo;

		vk::createSemaphore(Context::dev(), &semaphoreInfo, nullptr, &imageAquiredSemaphore);
		vk::createSemaphore(Context::dev(), &semaphoreInfo, nullptr, &renderingCompleteSemaphore);

		imageIndex = index;
		presentInfo
			.pImageIndices(&imageIndex)
			.pSwapchains(&info.swapChain)
			.swapchainCount(1)
			.pWaitSemaphores(&renderingCompleteSemaphore)
			.waitSemaphoreCount(1);

		vk::FenceCreateInfo fenceInfo;
		fenceInfo.flags(vk::FenceCreateFlagBits::eSignaled);

		presentToDrawBarrier
			.oldLayout(vk::ImageLayout::ePresentSrc)
			.newLayout(vk::ImageLayout::eColorAttachmentOptimal)
			.dstAccessMask(vk::AccessFlagBits::eColorAttachmentWrite | vk::AccessFlagBits::eColorAttachmentRead)
			.srcQueueFamilyIndex(Context::mainQueueFamilyIndex())
			.dstQueueFamilyIndex(Context::mainQueueFamilyIndex())
			.subresourceRange(vk::ImageSubresourceRange(vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1))
			.image(image);

		drawToPresentBarrier
			.oldLayout(vk::ImageLayout::eColorAttachmentOptimal)
			.newLayout(vk::ImageLayout::ePresentSrc)
			.srcAccessMask(vk::AccessFlagBits::eColorAttachmentWrite | vk::AccessFlagBits::eColorAttachmentRead)
			.srcQueueFamilyIndex(Context::mainQueueFamilyIndex())
			.dstQueueFamilyIndex(Context::mainQueueFamilyIndex())
			.subresourceRange(vk::ImageSubresourceRange(vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1))
			.image(image);

		vk::ImageMemoryBarrier defineImageBarrier;
		defineImageBarrier
			.oldLayout(vk::ImageLayout::eUndefined)
			.newLayout(vk::ImageLayout::ePresentSrc)
			.srcQueueFamilyIndex(Context::mainQueueFamilyIndex())
			.dstQueueFamilyIndex(Context::mainQueueFamilyIndex())
			.subresourceRange(vk::ImageSubresourceRange(vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1))
			.image(image);

		CommandBuffer buffer;
		buffer.beginRecording();
		
		transition(buffer, defineImageBarrier);

		buffer.endRecording();

		vk::CommandBuffer b = buffer;
		vk::SubmitInfo submitInfo;
		submitInfo
			.commandBufferCount(1)
			.pCommandBuffers(&b);

		vk::queueSubmit(Context::mainQueue(), 0, &submitInfo, nullptr);
		vk::queueWaitIdle(Context::mainQueue());
	}

	void Screen::Frame::present()
	{
		dc(vk::queuePresentKHR(Context::mainQueue(), presentInfo));
	}

	void Screen::Frame::transitionForDraw(vk::CommandBuffer buffer)
	{
		transition(buffer, presentToDrawBarrier);
	}

	void Screen::Frame::transitionForPresent(vk::CommandBuffer buffer)
	{
		transition(buffer, drawToPresentBarrier);
	}

	void Screen::Frame::transition(vk::CommandBuffer buffer, const vk::ImageMemoryBarrier & barrier)
	{
		vk::cmdPipelineBarrier(buffer,
			vk::PipelineStageFlagBits::eAllCommands,
			vk::PipelineStageFlagBits::eTopOfPipe,
			vk::DependencyFlags(),
			0, nullptr,
			0, nullptr,
			1, &barrier);
	}

	void Screen::Frame::destroy()
	{
		vk::destroySemaphore(Context::dev(), renderingCompleteSemaphore, nullptr);
		vk::destroySemaphore(Context::dev(), imageAquiredSemaphore, nullptr);
		vk::destroyFramebuffer(Context::dev(), frameBuffer, nullptr);
		vk::destroyImageView(Context::dev(), view, nullptr);
	}
}