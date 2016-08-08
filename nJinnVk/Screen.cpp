#include "stdafx.hpp"
#include "Screen.hpp"


#include <cstdio>
#include <algorithm>
#include <chrono>
#include <iostream>
#include "Application.hpp"
#include "Context.hpp"
#include "CommandBuffer.hpp"
#include "Config.hpp"
#include "Debug.hpp"

namespace nJinn {
	LRESULT CALLBACK WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

	Screen * screen;

	Screen::Screen(uint32_t width, uint32_t height) :
		mWindowHandle(nullptr),
		frameCount(config.getValue<uint32_t>("swapchain.backBufferCount")),
		maxQueuedFrames(config.getValue<uint32_t>("swapchain.maxQueuedFrames")),
		queueIndex(-1),
		mWidth(-1),
		mHeight(-1),
		mSwapChain(nullptr),
		mColorFormat(vk::Format::eUndefined),
		mRenderPass(nullptr),
		mSurface(nullptr),
		mColorSpace(vk::ColorSpaceKHR::eSrgbNonlinear),
		mFrames(nullptr),
		mCurrentFrameIndex(0),
		mCurrentFrame(nullptr),
		mCurrentAcquireFrameSemaphore(nullptr),
		mFences(nullptr),
		mTotalFrames(0)
	{
		WNDCLASSEX windowClass = { 0 };
		windowClass.cbSize = sizeof(WNDCLASSEX);
		windowClass.style = CS_HREDRAW | CS_VREDRAW;
		windowClass.lpfnWndProc = WindowProc;
		windowClass.hInstance = os::hInstance;
		windowClass.hCursor = LoadCursor(NULL, IDC_ARROW);
		windowClass.lpszClassName = L"WindowClass1";
		RegisterClassEx(&windowClass);

		RECT windowRect = { 0, 0, (LONG) width, (LONG) height };
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
			os::hInstance,
			NULL);		// We aren't using multiple windows, NULL.

		ShowWindow((HWND) mWindowHandle, os::nCmdShow);

		vk::Win32SurfaceCreateInfoKHR surfaceCreateInfo = {};
		surfaceCreateInfo.setHinstance(os::hInstance);
		surfaceCreateInfo.setHwnd((HWND) mWindowHandle);
		mSurface = context->inst().createWin32SurfaceKHR(surfaceCreateInfo);

		std::vector<vk::QueueFamilyProperties> queueProperties = context->physDev().getQueueFamilyProperties();
		for (int i = 0; i < queueProperties.size(); ++i) {
			if (queueProperties[i].queueFlags & vk::QueueFlagBits::eGraphics) {
				uint32_t supported = 0;
				supported = context->physDev().getSurfaceSupportKHR(i, mSurface);
				if (supported) {
					queueIndex = i;
					break;
				}
			}
		}

		assert(queueIndex != -1);

		std::vector<vk::SurfaceFormatKHR> surfaceFormats = context->physDev().getSurfaceFormatsKHR(mSurface);

		if (1 == surfaceFormats.size() && vk::Format::eUndefined == surfaceFormats[0].format) {
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
					if (it->format == f) break;
					++it;
				}
				if (surfaceFormats.end() != it) {
					mColorFormat = f;
					mColorSpace = it->colorSpace;
					break;
				}
			}
		}

		presentInfo
			.setPImageIndices(&mCurrentFrameIndex)
			.setPSwapchains(&mSwapChain)
			.setSwapchainCount(1)
			.setWaitSemaphoreCount(1);

		presentToDrawBarrier
			.setOldLayout(vk::ImageLayout::ePresentSrcKHR)
			.setNewLayout(vk::ImageLayout::eColorAttachmentOptimal)
			.setDstAccessMask(vk::AccessFlagBits::eColorAttachmentWrite | vk::AccessFlagBits::eColorAttachmentRead)
			.setSrcQueueFamilyIndex(VK_QUEUE_FAMILY_IGNORED)
			.setDstQueueFamilyIndex(VK_QUEUE_FAMILY_IGNORED)
			.setSubresourceRange(vk::ImageSubresourceRange(vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1));

		drawToPresentBarrier
			.setOldLayout(vk::ImageLayout::eColorAttachmentOptimal)
			.setNewLayout(vk::ImageLayout::ePresentSrcKHR)
			.setSrcAccessMask(vk::AccessFlagBits::eColorAttachmentWrite | vk::AccessFlagBits::eColorAttachmentRead)
			.setSrcQueueFamilyIndex(VK_QUEUE_FAMILY_IGNORED)
			.setDstQueueFamilyIndex(VK_QUEUE_FAMILY_IGNORED)
			.setSubresourceRange(vk::ImageSubresourceRange(vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1));

		resize(width, height);
	}

	Screen::~Screen()
	{
		for (uint32_t i = 0; i < frameCount; ++i) {
			mFrames[i].destroy();
		}
		for (uint32_t i = 0; i < maxQueuedFrames; ++i) {
			context->dev().destroyFence(mFences[i]);
		}
		delete[] mFrames;
		delete[] mFences;
		context->dev().destroyRenderPass(mRenderPass);
		context->dev().destroySwapchainKHR(mSwapChain);
		context->inst().destroySurfaceKHR(mSurface);
	}

	void Screen::resize(uint32_t width, uint32_t height)
	{
		vk::SwapchainKHR oldSwapChain = mSwapChain;

		vk::SurfaceCapabilitiesKHR surfaceCaps;
		surfaceCaps = context->physDev().getSurfaceCapabilitiesKHR(mSurface);

		std::vector<vk::PresentModeKHR> presentModes = context->physDev().getSurfacePresentModesKHR(mSurface);

		vk::Extent2D swapChainExtent;
		if (surfaceCaps.currentExtent.width == -1) {
			swapChainExtent.setWidth(width);
			swapChainExtent.setHeight(height);
		} else {
			swapChainExtent = surfaceCaps.currentExtent;
		}
		mWidth = swapChainExtent.width;
		mHeight = swapChainExtent.height;

		vk::PresentModeKHR presentMode = vk::PresentModeKHR::eFifo;
		for (auto pm : presentModes) {
			if (pm == vk::PresentModeKHR::eMailbox) {
				presentMode = pm;
				break;
			}
		}

		vk::SwapchainCreateInfoKHR swapChainInfo;
		swapChainInfo
			.setSurface(mSurface)
			.setMinImageCount(frameCount)
			.setImageFormat(mColorFormat)
			.setImageColorSpace(mColorSpace)
			.setImageExtent(swapChainExtent)
			.setImageUsage(vk::ImageUsageFlagBits::eColorAttachment)
			.setPreTransform(vk::SurfaceTransformFlagBitsKHR::eIdentity)
			.setImageArrayLayers(1)
			.setQueueFamilyIndexCount(0)
			.setImageSharingMode(vk::SharingMode::eExclusive)
			.setPQueueFamilyIndices(nullptr)
			.setPresentMode(presentMode)
			.setOldSwapchain(oldSwapChain)
			.setClipped(true)
			.setCompositeAlpha(vk::CompositeAlphaFlagBitsKHR::eOpaque);

		mSwapChain = context->dev().createSwapchainKHR(swapChainInfo);

		if (oldSwapChain) {
			context->dev().destroySwapchainKHR(oldSwapChain);
			for (uint32_t i = 0; i < frameCount; ++i) {
				mFrames[i].destroy();
			}
			for (uint32_t i = 0; i < maxQueuedFrames; ++i) {
				context->dev().destroyFence(mFences[i]);
			}
			delete[] mFrames;
			delete[] mFences;
		}

		std::vector<vk::Image> images = context->dev().getSwapchainImagesKHR(mSwapChain);
		assert(frameCount == images.size());

		vk::AttachmentDescription attachment;
		attachment
			.setFormat(mColorFormat)
			.setSamples(vk::SampleCountFlagBits::e1)
			.setLoadOp(vk::AttachmentLoadOp::eClear)
			.setStoreOp(vk::AttachmentStoreOp::eStore)
			.setStencilLoadOp(vk::AttachmentLoadOp::eDontCare)
			.setStencilStoreOp(vk::AttachmentStoreOp::eDontCare)
			.setInitialLayout(vk::ImageLayout::eColorAttachmentOptimal)
			.setFinalLayout(vk::ImageLayout::eColorAttachmentOptimal);
			
		vk::AttachmentReference attachmentReference(0, vk::ImageLayout::eColorAttachmentOptimal);

		vk::SubpassDescription subpass;
		subpass
			.setPipelineBindPoint(vk::PipelineBindPoint::eGraphics)
			.setColorAttachmentCount(1)
			.setPColorAttachments(&attachmentReference);


		vk::RenderPassCreateInfo renderPassInfo;
		renderPassInfo
			.setAttachmentCount(1)
			.setPAttachments(&attachment)
			.setSubpassCount(1)
			.setPSubpasses(&subpass);

		mRenderPass = context->dev().createRenderPass(renderPassInfo);

		vk::ImageViewCreateInfo viewInfo;
		viewInfo
			.setFormat(mColorFormat)
			.setComponents(vk::ComponentMapping())
			.setSubresourceRange(vk::ImageSubresourceRange(vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1))
			.setViewType(vk::ImageViewType::e2D);

		vk::FramebufferCreateInfo framebufferInfo;
		framebufferInfo
			.setAttachmentCount(1)
			.setRenderPass(renderPass())
			.setHeight(mHeight)
			.setWidth(mWidth)
			.setLayers(1);

		mFrames = new Frame[frameCount];

		for (size_t i = 0; i < frameCount; ++i) {
			Frame & frame = mFrames[i];
			viewInfo.setImage(images[i]);
			frame.view = context->dev().createImageView(viewInfo);
			framebufferInfo.setPAttachments(&frame.view);
			frame.frameBuffer = context->dev().createFramebuffer(framebufferInfo);
			frame.imageAquiredSemaphore = context->dev().createSemaphore(vk::SemaphoreCreateInfo());
			frame.renderingCompleteSemaphore = context->dev().createSemaphore(vk::SemaphoreCreateInfo());
			frame.image = images[i];
		}

		mFences = new vk::Fence[maxQueuedFrames];
		vk::FenceCreateInfo info;
		info.flags = vk::FenceCreateFlagBits::eSignaled;
		for (size_t i = 0; i < maxQueuedFrames; ++i) {
			mFences[i] = context->dev().createFence(info);
			//info.flags = vk::FenceCreateFlags();
		}
		
		setCurrentFrame(0);

		vk::ImageMemoryBarrier defineImageBarrier;
		defineImageBarrier
			.setOldLayout(vk::ImageLayout::eUndefined)
			.setNewLayout(vk::ImageLayout::ePresentSrcKHR)
			.setSrcQueueFamilyIndex(VK_QUEUE_FAMILY_IGNORED)
			.setDstQueueFamilyIndex(VK_QUEUE_FAMILY_IGNORED)
			.setSubresourceRange(vk::ImageSubresourceRange(vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1));

		CommandBuffer buffer;
		buffer.beginRecording();
		for (uint32_t i = 0; i < frameCount; ++i) {
			defineImageBarrier.setImage(images[i]);
			buffer->pipelineBarrier(
				vk::PipelineStageFlagBits::eAllCommands,
				vk::PipelineStageFlagBits::eTopOfPipe,
				vk::DependencyFlags(),
				0, nullptr,
				0, nullptr,
				1, &defineImageBarrier);
		}
		buffer.endRecording();

		vk::SubmitInfo submitInfo;
		submitInfo
			.setCommandBufferCount(1)
			.setPCommandBuffers(buffer.get());

		context->mainQueue().submit(1, &submitInfo, nullptr);
		context->mainQueue().waitIdle();
	}

	void Screen::present()
	{
		context->mainQueue().submit(0, nullptr, mFences[mTotalFrames % maxQueuedFrames]);
		presentInfo.setPWaitSemaphores(&mCurrentFrame->renderingCompleteSemaphore);
		context->mainQueue().presentKHR(presentInfo);
		++mTotalFrames;
	}

	void Screen::acquireFrame()
	{
		vk::Fence * pFence = mFences + (mTotalFrames % maxQueuedFrames);
		// leave this loop as is
		// vulkan tends to return from waitForFences, even if fences are not signaled
		do{
			context->dev().waitForFences(1, pFence, true, std::numeric_limits<uint64_t>::max());
		}while (context->dev().getFenceStatus(*pFence) == vk::Result::eNotReady);
		context->dev().resetFences(1, pFence);

		mCurrentAcquireFrameSemaphore = mCurrentFrame->imageAquiredSemaphore;
		acquireFrameIndex(mCurrentAcquireFrameSemaphore);
	}

	void Screen::acquireFrameIndex(vk::Semaphore signalSemaphore, vk::Fence signalFence)
	{
		uint32_t imageIndex = context->dev().acquireNextImageKHR(mSwapChain, -1, signalSemaphore, signalFence).value;
		setCurrentFrame(imageIndex);
	}

	void Screen::setCurrentFrame(uint32_t index) {
		mCurrentFrameIndex = index;
		mCurrentFrame = &mFrames[mCurrentFrameIndex];
	}

	void Screen::transitionForDraw(vk::CommandBuffer cmdbuf)
	{
		presentToDrawBarrier.setImage(mCurrentFrame->image);
		cmdbuf.pipelineBarrier(
			vk::PipelineStageFlagBits::eAllCommands,
			vk::PipelineStageFlagBits::eTopOfPipe,
			vk::DependencyFlags(),
			0, nullptr,
			0, nullptr,
			1, &presentToDrawBarrier);
	}

	void Screen::transitionForPresent(vk::CommandBuffer cmdbuf)
	{
		drawToPresentBarrier.setImage(mCurrentFrame->image);
		cmdbuf.pipelineBarrier(
			vk::PipelineStageFlagBits::eAllCommands,
			vk::PipelineStageFlagBits::eTopOfPipe,
			vk::DependencyFlags(),
			0, nullptr,
			0, nullptr,
			1, &drawToPresentBarrier);
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

	void Screen::Frame::destroy()
	{
		context->dev().destroySemaphore(renderingCompleteSemaphore);
		context->dev().destroySemaphore(imageAquiredSemaphore);
		context->dev().destroyFramebuffer(frameBuffer);
		context->dev().destroyImageView(view);
	}
}