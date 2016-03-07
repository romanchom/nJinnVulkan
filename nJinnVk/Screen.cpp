#include "stdafx.hpp"
#include "Screen.hpp"


#include <cstdio>
#include <algorithm>
#include <chrono>
#include <iostream>
#include "Application.hpp"
#include "Context.hpp"
#include "CommandBuffer.hpp"

namespace nJinn {
	LRESULT CALLBACK WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

	Screen::Screen(uint32_t width, uint32_t height) :
		mWindowHandle(nullptr),
		queueIndex(-1),
		mSwapChain(nullptr),
		mRenderPass(nullptr),
		mSurface(nullptr),
		mWidth(-1),
		mHeight(-1),
		mCurrentFrameIndex(0),
		mCurrentFrame(mFrames + mCurrentFrameIndex),
		mCurrentAcquireFrameSemaphore(nullptr),
		mCurrentFence(0),
		mTotalFrames(0)
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
		dc(vk::createWin32SurfaceKHR(Context::inst(), &surfaceCreateInfo, nullptr, &mSurface));

		std::vector<vk::QueueFamilyProperties> queueProperties = vk::getPhysicalDeviceQueueFamilyProperties(Context::physDev());
		for (int i = 0; i < queueProperties.size(); ++i) {
			if (queueProperties[i].queueFlags() & vk::QueueFlagBits::eGraphics) {
				uint32_t supported = 0;
				dc(vk::getPhysicalDeviceSurfaceSupportKHR(Context::physDev(), i, mSurface, supported));
				if (supported) {
					queueIndex = i;
					break;
				}
			}
		}

		assert(queueIndex != -1);

		std::vector<vk::SurfaceFormatKHR> surfaceFormats;
		dc(vk::getPhysicalDeviceSurfaceFormatsKHR(Context::physDev(), mSurface, surfaceFormats));

		if (1 == surfaceFormats.size() && vk::Format::eUndefined == surfaceFormats[0].format()) {
			mColorFormat = vk::Format::eB8G8R8A8Srgb;
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
					mColorFormat = f;
					mColorSpace = it->colorSpace();
					break;
				}
			}
		}

		resize(width, height);
	}

	Screen::~Screen()
	{
		vk::destroyRenderPass(Context::dev(), mRenderPass, nullptr);
		vk::destroySurfaceKHR(Context::inst(), mSurface, nullptr);
	}

	void Screen::resize(uint32_t width, uint32_t height)
	{
		vk::SwapchainKHR oldSwapChain = mSwapChain;

		vk::SurfaceCapabilitiesKHR surfaceCaps;
		dc(vk::getPhysicalDeviceSurfaceCapabilitiesKHR(Context::physDev(), mSurface, surfaceCaps));

		std::vector<vk::PresentModeKHR> presentModes;
		vk::getPhysicalDeviceSurfacePresentModesKHR(Context::physDev(), mSurface, presentModes);

		vk::Extent2D swapChainExtent;
		if (surfaceCaps.currentExtent().width() == -1) {
			swapChainExtent.width(width);
			swapChainExtent.height(height);
		} else {
			swapChainExtent = surfaceCaps.currentExtent();
		}
		mWidth = swapChainExtent.width();
		mHeight = swapChainExtent.height();

		vk::PresentModeKHR presentMode = vk::PresentModeKHR::eVkPresentModeFifoKhr;
		for (auto pm : presentModes) {
			if (pm == vk::PresentModeKHR::eVkPresentModeMailboxKhr) {
				presentMode = pm;
				break;
			}
		}

		vk::SwapchainCreateInfoKHR swapChainInfo;
		swapChainInfo
			.surface(mSurface)
			.minImageCount(frameCount)
			.imageFormat(mColorFormat)
			.imageColorSpace(mColorSpace)
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

		dc(vk::createSwapchainKHR(Context::dev(), &swapChainInfo, nullptr, &mSwapChain));

		if (oldSwapChain) {
			vk::destroySwapchainKHR(Context::dev(), oldSwapChain, nullptr);
		}

		

		std::vector<vk::Image> images;
		dc(vk::getSwapchainImagesKHR(Context::dev(), mSwapChain, images));

		assert(frameCount == images.size());

		vk::AttachmentDescription attachment;
		attachment
			.format(mColorFormat)
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

		dc(vk::createRenderPass(Context::dev(), &renderPassInfo, nullptr, &mRenderPass));

		for (size_t i = 0; i < frameCount; ++i) {
			mFrames[i].create(*this, i, images[i]);
		}
		vk::FenceCreateInfo fenceInfo;
		fenceInfo.flags(vk::FenceCreateFlagBits::eSignaled);
		for (size_t i = 0; i < maxQueuedFrames; ++i) {
			dc(vk::createFence(Context::dev(), &fenceInfo, nullptr, mFences + i));
			fenceInfo.flags(vk::FenceCreateFlags());
		}

		acquireFrameIndex();
	}

	void Screen::present()
	{
		vk::queueSubmit(Context::mainQueue(), 0, nullptr, mFences[mCurrentFence]);
		mCurrentFrame->present();
		++mTotalFrames;
	}

	void Screen::acquireFrame()
	{
		vk::Fence * pFence = &mFences[mCurrentFence];
		vk::waitForFences(Context::dev(), 1, pFence, 1, -1);
		vk::resetFences(Context::dev(), 1, pFence);

		mCurrentAcquireFrameSemaphore = mCurrentFrame->waitingSemaphore();
		acquireFrameIndex(mCurrentAcquireFrameSemaphore);
		++mCurrentFence %= maxQueuedFrames;
	}

	void Screen::acquireFrameIndex(vk::Semaphore signalSemaphore)
	{
		uint32_t imageIndex;
		dc(vk::acquireNextImageKHR(Context::dev(), mSwapChain, -1, signalSemaphore, nullptr, imageIndex));
		mCurrentFrameIndex = imageIndex;
		mCurrentFrame = mFrames + mCurrentFrameIndex;
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
		frameBuffer(nullptr)
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
			.format(info.mColorFormat)
			.components(vk::ComponentMapping())
			.subresourceRange(vk::ImageSubresourceRange(
				vk::ImageAspectFlagBits::eColor,
				0, 1, 0, 1))
			.viewType(vk::ImageViewType::e2D)
			.image(image);

		dc(vk::createImageView(Context::dev(), &viewInfo, nullptr, &view));

		vk::FramebufferCreateInfo framebufferInfo;
		framebufferInfo
			.attachmentCount(1)
			.pAttachments(&view)
			.renderPass(info.renderPass())
			.height(info.mHeight)
			.width(info.mWidth)
			.layers(1);

		vk::createFramebuffer(Context::dev(), &framebufferInfo, nullptr, &frameBuffer);

		vk::SemaphoreCreateInfo semaphoreInfo;

		imageIndex = index;
		presentInfo
			.pImageIndices(&imageIndex)
			.pSwapchains(&info.mSwapChain)
			.swapchainCount(1)
			.pWaitSemaphores(renderingCompleteSemaphore.get())
			.waitSemaphoreCount(1);

		vk::FenceCreateInfo fenceInfo;
		fenceInfo.flags(vk::FenceCreateFlagBits::eSignaled);

		presentToDrawBarrier
			.oldLayout(vk::ImageLayout::ePresentSrcKhr)
			.newLayout(vk::ImageLayout::eColorAttachmentOptimal)
			.dstAccessMask(vk::AccessFlagBits::eColorAttachmentWrite | vk::AccessFlagBits::eColorAttachmentRead)
			.srcQueueFamilyIndex(VK_QUEUE_FAMILY_IGNORED)
			.dstQueueFamilyIndex(VK_QUEUE_FAMILY_IGNORED)
			.subresourceRange(vk::ImageSubresourceRange(vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1))
			.image(image);

		drawToPresentBarrier
			.oldLayout(vk::ImageLayout::eColorAttachmentOptimal)
			.newLayout(vk::ImageLayout::ePresentSrcKhr)
			.srcAccessMask(vk::AccessFlagBits::eColorAttachmentWrite | vk::AccessFlagBits::eColorAttachmentRead)
			.srcQueueFamilyIndex(VK_QUEUE_FAMILY_IGNORED)
			.dstQueueFamilyIndex(VK_QUEUE_FAMILY_IGNORED)
			.subresourceRange(vk::ImageSubresourceRange(vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1))
			.image(image);

		vk::ImageMemoryBarrier defineImageBarrier;
		defineImageBarrier
			.oldLayout(vk::ImageLayout::eUndefined)
			.newLayout(vk::ImageLayout::ePresentSrcKhr)
			.srcQueueFamilyIndex(VK_QUEUE_FAMILY_IGNORED)
			.dstQueueFamilyIndex(VK_QUEUE_FAMILY_IGNORED)
			.subresourceRange(vk::ImageSubresourceRange(vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1))
			.image(image);

		CommandBuffer buffer;
		buffer.beginRecording();
		transition(buffer, defineImageBarrier);
		buffer.endRecording();

		vk::SubmitInfo submitInfo;
		submitInfo
			.commandBufferCount(1)
			.pCommandBuffers(buffer.get());

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