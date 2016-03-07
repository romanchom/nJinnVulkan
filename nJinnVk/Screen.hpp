#pragma once

#include <vulkan.hpp>

#include "Semaphore.hpp"

namespace nJinn
{
	class Screen {
		class Frame {
		public:
			Frame();
			~Frame();
			Frame(const Frame & that) = delete;
			Frame & operator=(const Frame & that) = delete;
			void create(const Screen & info, size_t index, vk::Image img);
			void present();
			void transitionForDraw(vk::CommandBuffer buffer);
			void transitionForPresent(vk::CommandBuffer buffer);
			vk::Semaphore waitingSemaphore() const { return imageAquiredSemaphore; }
			vk::Semaphore signalingSemaphore() const { return renderingCompleteSemaphore; }
			vk::Framebuffer framebuffer() const { return frameBuffer; }
		private:
			void transition(vk::CommandBuffer buffer, const vk::ImageMemoryBarrier & barrier);
			void destroy();
			vk::Image image;
			vk::ImageView view;
			vk::Framebuffer frameBuffer;
			Semaphore imageAquiredSemaphore;
			Semaphore renderingCompleteSemaphore;
			vk::PresentInfoKHR presentInfo;
			vk::ImageMemoryBarrier presentToDrawBarrier;
			vk::ImageMemoryBarrier drawToPresentBarrier;
			uint32_t imageIndex;
		};

		enum {
			frameCount = 2,
			maxQueuedFrames = 2
		};

		bool shouldClose();
		void acquireFrameIndex(vk::Semaphore signalSemaphore = nullptr);

		void * mWindowHandle;

		size_t queueIndex;

		vk::SwapchainKHR mSwapChain;
		vk::Format mColorFormat;
		vk::RenderPass mRenderPass;
		vk::SurfaceKHR mSurface;
		vk::ColorSpaceKHR mColorSpace;
		uint32_t mWidth;
		uint32_t mHeight;
		
		Frame mFrames[frameCount];
		size_t mCurrentFrameIndex;
		Frame * mCurrentFrame;
		vk::Semaphore mCurrentAcquireFrameSemaphore;
		vk::Fence mFences[maxQueuedFrames];
		size_t mCurrentFence;
		size_t mTotalFrames;

		friend class Application;
	public:
		Screen(uint32_t width, uint32_t height);
		~Screen();
		void resize(uint32_t width, uint32_t height);
		void present();
		void acquireFrame();
		vk::Semaphore waitingSemaphore() const { return mCurrentAcquireFrameSemaphore; }
		vk::Semaphore renderCompleteSemaphore() const { return mCurrentFrame->signalingSemaphore(); }
		vk::RenderPass renderPass() const { return mRenderPass; }
		vk::Framebuffer framebuffer() const { return mCurrentFrame->framebuffer(); }
		void transitionForDraw(vk::CommandBuffer cmdbuf) { mCurrentFrame->transitionForDraw(cmdbuf); }
		void transitionForPresent(vk::CommandBuffer cmdbuf) { mCurrentFrame->transitionForPresent(cmdbuf); }
		uint32_t width() { return mWidth; };
		uint32_t height() { return mHeight; };
	};
}
