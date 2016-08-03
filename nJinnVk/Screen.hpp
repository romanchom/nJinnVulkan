#pragma once

#include <vulkan.hpp>

#include "Semaphore.hpp"
#include "Fence.hpp"

namespace nJinn
{
	class Screen {
	private:
		struct Frame {
			void destroy();
			vk::Image image;
			vk::ImageView view;
			vk::Framebuffer frameBuffer;
			vk::Semaphore imageAquiredSemaphore;
			vk::Semaphore renderingCompleteSemaphore;
		};

		void * mWindowHandle;
		uint32_t frameCount;
		uint32_t maxQueuedFrames;
		size_t queueIndex;
		uint32_t mWidth;
		uint32_t mHeight;

		vk::SwapchainKHR mSwapChain;
		vk::Format mColorFormat;
		vk::RenderPass mRenderPass;
		vk::SurfaceKHR mSurface;
		vk::ColorSpaceKHR mColorSpace;

		vk::PresentInfoKHR presentInfo;
		vk::ImageMemoryBarrier presentToDrawBarrier;
		vk::ImageMemoryBarrier drawToPresentBarrier;
		
		Frame * mFrames;
		uint32_t mCurrentFrameIndex;
		Frame * mCurrentFrame;
		vk::Semaphore mCurrentAcquireFrameSemaphore;
		vk::Fence * mFences;
		size_t mTotalFrames;

		bool shouldClose();
		void acquireFrameIndex(vk::Semaphore signalSemaphore = nullptr, vk::Fence signalFence = nullptr);
		void setCurrentFrame(uint32_t index);
	public:
		Screen(uint32_t width, uint32_t height);
		~Screen();
		void resize(uint32_t width, uint32_t height);
		void present();
		void acquireFrame();
		vk::Semaphore waitingSemaphore() const { return mCurrentAcquireFrameSemaphore; }
		vk::Semaphore renderCompleteSemaphore() const { return mCurrentFrame->renderingCompleteSemaphore; }
		vk::RenderPass renderPass() const { return mRenderPass; }
		vk::Framebuffer framebuffer() const { return mCurrentFrame->frameBuffer; }
		void transitionForDraw(vk::CommandBuffer cmdbuf);
		void transitionForPresent(vk::CommandBuffer cmdbuf);
		uint32_t width() { return mWidth; };
		uint32_t height() { return mHeight; };

		friend class Application;
	};
}
