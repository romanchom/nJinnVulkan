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
		};

		void * mWindowHandle;
		uint32_t mFrameCount;
		size_t queueIndex;
		uint32_t mWidth;
		uint32_t mHeight;

		vk::SwapchainKHR mSwapChain;
		vk::Format mColorFormat;
		vk::SurfaceKHR mSurface;
		vk::ColorSpaceKHR mColorSpace;

		vk::PresentInfoKHR presentInfo;
		vk::ImageMemoryBarrier presentToDrawBarrier;
		vk::ImageMemoryBarrier drawToPresentBarrier;
		
		Frame * mFrames;
		uint32_t mCurrentFrameIndex;
		Frame * mCurrentFrame;
		size_t mTotalFrames;

		bool shouldClose();
		void setCurrentFrame(uint32_t index);
	public:
		Screen(uint32_t width, uint32_t height);
		~Screen();
		void resize(uint32_t width, uint32_t height);
		void present(vk::Semaphore renderingCompleteSemaphore);
		void acquireFrame(vk::Semaphore frameAquiredSemaphore);
		void transitionForDraw(vk::CommandBuffer cmdbuf);
		void transitionForPresent(vk::CommandBuffer cmdbuf);
		uint32_t width() { return mWidth; };
		uint32_t height() { return mHeight; };
		uint32_t currentFrameIndex() const { return mCurrentFrameIndex; }
		vk::ImageView getImageView(uint32_t index) { return mFrames[index].view; }
		vk::Image getImage(uint32_t index) { return mFrames[index].image; }
		uint32_t frameCount() const noexcept { return mFrameCount; }


		friend class Application;
	};

	extern Screen * screen;
}
