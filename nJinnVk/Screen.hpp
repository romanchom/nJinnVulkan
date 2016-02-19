#pragma once

#include <vulkan.hpp>

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
		private:
			void destroy();
			vk::Image image;
			vk::ImageView view;
			vk::Framebuffer frameBuffer;
			vk::Semaphore imageAquiredSemaphore;
			vk::Semaphore renderingCompleteSemaphore;
			vk::PresentInfoKHR presentInfo;
			uint32_t imageIndex;
			friend class Screen;
		};

		enum {
			maxQueuedFrames = 2
		};
	public:
		Screen(uint32_t width, uint32_t height);
		~Screen();
		void resize(uint32_t width, uint32_t height);
		void present();
		void acquireFrame();
	private:
		bool shouldClose();
		static void allocateConsole();

		void * mWindowHandle;

		size_t queueIndex;

		vk::SwapchainKHR swapChain;
		vk::Format colorFormat;
		vk::RenderPass renderPass;
		vk::SurfaceKHR surface;
		vk::ColorSpaceKHR colorSpace;
		uint32_t width;
		uint32_t height;
		
		Frame frames[3];
		size_t currentFrameIndex;
		Frame * currentFrame;
		vk::Semaphore currentAcquireFrameSemaphore;
		vk::Fence fences[maxQueuedFrames];
		size_t currentFence;


		friend class Application;
	};
}
