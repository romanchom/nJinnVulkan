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
		surfaceCreateInfo.setHinstance(Application::shInstance);
		surfaceCreateInfo.setHwnd((HWND) mWindowHandle);
		mSurface = Context::inst().createWin32SurfaceKHR(surfaceCreateInfo);

		std::vector<vk::QueueFamilyProperties> queueProperties = Context::physDev().getQueueFamilyProperties();
		for (int i = 0; i < queueProperties.size(); ++i) {
			if (queueProperties[i].queueFlags & vk::QueueFlagBits::eGraphics) {
				uint32_t supported = 0;
				supported = Context::physDev().getSurfaceSupportKHR(i, mSurface);
				if (supported) {
					queueIndex = i;
					break;
				}
			}
		}

		assert(queueIndex != -1);

		std::vector<vk::SurfaceFormatKHR> surfaceFormats = Context::physDev().getSurfaceFormatsKHR(mSurface);

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

		resize(width, height);
	}

	Screen::~Screen()
	{
		Context::dev().destroyRenderPass(mRenderPass);
		Context::inst().destroySurfaceKHR(mSurface);
	}

	void Screen::resize(uint32_t width, uint32_t height)
	{
		vk::SwapchainKHR oldSwapChain = mSwapChain;

		vk::SurfaceCapabilitiesKHR surfaceCaps;
		surfaceCaps = Context::physDev().getSurfaceCapabilitiesKHR(mSurface);

		std::vector<vk::PresentModeKHR> presentModes = Context::physDev().getSurfacePresentModesKHR(mSurface);

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

		mSwapChain = Context::dev().createSwapchainKHR(swapChainInfo);

		if (oldSwapChain) {
			Context::dev().destroySwapchainKHR(oldSwapChain);
		}

		

		std::vector<vk::Image> images = Context::dev().getSwapchainImagesKHR(mSwapChain);

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

		mRenderPass = Context::dev().createRenderPass(renderPassInfo);

		for (size_t i = 0; i < frameCount; ++i) {
			mFrames[i].create(*this, i, images[i]);
		}
		vk::FenceCreateInfo fenceInfo;
		fenceInfo.setFlags(vk::FenceCreateFlagBits::eSignaled);
		for (size_t i = 0; i < maxQueuedFrames; ++i) {
			mFences[i] = Context::dev().createFence(fenceInfo);
			fenceInfo.setFlags(vk::FenceCreateFlags());
		}

		setCurrentFrame(0);
		//mCurrentAcquireFrameSemaphore = mCurrentFrame->waitingSemaphore();
		//vk::Fence tempFence = Context::dev().createFence(vk::FenceCreateInfo());
		//acquireFrameIndex(waitingSemaphore());
		//Context::dev().waitForFences(1, &tempFence, 1, )
	}

	void Screen::present()
	{
		Context::mainQueue().submit(0, nullptr, mFences[mCurrentFence]);
		mCurrentFrame->present();
		++mTotalFrames;
	}

	void Screen::acquireFrame()
	{
		vk::Fence * pFence = &mFences[mCurrentFence];
		assert(Context::dev().waitForFences(1, pFence, true, -1) == vk::Result::eSuccess);
		Context::dev().resetFences(1, pFence);

		mCurrentAcquireFrameSemaphore = mCurrentFrame->waitingSemaphore();
		acquireFrameIndex(mCurrentAcquireFrameSemaphore);
		//++mCurrentFence %= maxQueuedFrames;
	}

	void Screen::acquireFrameIndex(vk::Semaphore signalSemaphore, vk::Fence signalFence)
	{
		uint32_t imageIndex = Context::dev().acquireNextImageKHR(mSwapChain, -1, signalSemaphore, signalFence).value;
		setCurrentFrame(imageIndex);
	}

	void Screen::setCurrentFrame(uint32_t index) {
		mCurrentFrameIndex = index;
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
			.setFormat(info.mColorFormat)
			.setComponents(vk::ComponentMapping())
			.setSubresourceRange(vk::ImageSubresourceRange(
				vk::ImageAspectFlagBits::eColor,
				0, 1, 0, 1))
			.setViewType(vk::ImageViewType::e2D)
			.setImage(image);

		view = Context::dev().createImageView(viewInfo);

		vk::FramebufferCreateInfo framebufferInfo;
		framebufferInfo
			.setAttachmentCount(1)
			.setPAttachments(&view)
			.setRenderPass(info.renderPass())
			.setHeight(info.mHeight)
			.setWidth(info.mWidth)
			.setLayers(1);

		frameBuffer = Context::dev().createFramebuffer(framebufferInfo);

		vk::SemaphoreCreateInfo semaphoreInfo;

		imageIndex = index;
		presentInfo
			.setPImageIndices(&imageIndex)
			.setPSwapchains(&info.mSwapChain)
			.setSwapchainCount(1)
			.setPWaitSemaphores(renderingCompleteSemaphore.get())
			.setWaitSemaphoreCount(1);

		vk::FenceCreateInfo fenceInfo;
		fenceInfo.setFlags(vk::FenceCreateFlagBits::eSignaled);

		presentToDrawBarrier
			.setOldLayout(vk::ImageLayout::ePresentSrcKHR)
			.setNewLayout(vk::ImageLayout::eColorAttachmentOptimal)
			//.setDstAccessMask(vk::AccessFlagBits::eColorAttachmentWrite | vk::AccessFlagBits::eColorAttachmentRead)
			.setSrcQueueFamilyIndex(VK_QUEUE_FAMILY_IGNORED)
			.setDstQueueFamilyIndex(VK_QUEUE_FAMILY_IGNORED)
			.setSubresourceRange(vk::ImageSubresourceRange(vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1))
			.setImage(image);

		drawToPresentBarrier
			.setOldLayout(vk::ImageLayout::eColorAttachmentOptimal)
			.setNewLayout(vk::ImageLayout::ePresentSrcKHR)
			//.setSrcAccessMask(vk::AccessFlagBits::eColorAttachmentWrite | vk::AccessFlagBits::eColorAttachmentRead)
			//.setDstAccessMask(vk::AccessFlagBits::eColorAttachmentRead)
			.setSrcQueueFamilyIndex(VK_QUEUE_FAMILY_IGNORED)
			.setDstQueueFamilyIndex(VK_QUEUE_FAMILY_IGNORED)
			.setSubresourceRange(vk::ImageSubresourceRange(vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1))
			.setImage(image);

		vk::ImageMemoryBarrier defineImageBarrier;
		defineImageBarrier
			.setOldLayout(vk::ImageLayout::eUndefined)
			.setNewLayout(vk::ImageLayout::ePresentSrcKHR)
			//.setSrcAccessMask(vk::AccessFlagBits::eColorAttachmentWrite | vk::AccessFlagBits::eColorAttachmentRead)
			.setDstAccessMask( vk::AccessFlagBits::eColorAttachmentRead)
			.setSrcQueueFamilyIndex(VK_QUEUE_FAMILY_IGNORED)
			.setDstQueueFamilyIndex(VK_QUEUE_FAMILY_IGNORED)
			.setSubresourceRange(vk::ImageSubresourceRange(vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1))
			.setImage(image);


		CommandBuffer buffer;
		buffer.beginRecording();
		transition(buffer, defineImageBarrier);
		buffer.endRecording();

		vk::SubmitInfo submitInfo;
		submitInfo
			.setCommandBufferCount(1)
			.setPCommandBuffers(buffer.get());

		Context::mainQueue().submit(1, &submitInfo, nullptr);
		Context::mainQueue().waitIdle();
	}

	void Screen::Frame::present()
	{
		Context::mainQueue().presentKHR(presentInfo);
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
		buffer.pipelineBarrier(
			vk::PipelineStageFlagBits::eAllCommands,
			vk::PipelineStageFlagBits::eTopOfPipe,
			vk::DependencyFlags(),
			0, nullptr,
			0, nullptr,
			1, &barrier);
	}

	void Screen::Frame::destroy()
	{
		Context::dev().destroySemaphore(renderingCompleteSemaphore);
		Context::dev().destroySemaphore(imageAquiredSemaphore);
		Context::dev().destroyFramebuffer(frameBuffer);
		Context::dev().destroyImageView(view);
	}
}