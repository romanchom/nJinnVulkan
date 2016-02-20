#include "stdafx.hpp"
#include "Application.hpp"

#include <chrono>

#include "Config.hpp"
#include "Screen.hpp"
#include "Context.hpp"
#include "CommandBuffer.hpp"

namespace nJinn {
	GameBase * Application::mGame = nullptr;
	HINSTANCE Application::shInstance = 0;
	int Application::snCmdShow = 1;
	class Screen * Application::sScreen = nullptr;

	typedef std::chrono::high_resolution_clock clock;

	void Application::run()
	{
		auto begin = clock::now();
		mGame->onInitialize();
		size_t frame = 0;
		CommandBuffer buf;
		vk::ClearColorValue color;
		color.float32({ 1, 0, 1, 1 });
		float val = 0;
		while (true) {
			if (sScreen->shouldClose()) break;
			sScreen->acquireFrame();

			buf.beginRecording();

			sScreen->currentFrame->transitionForDraw(buf);
			val = 1 - val;
			color.float32({ val, 0, 0, 0 });
			vk::ImageSubresourceRange range(vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1);
			vk::cmdClearColorImage(buf, sScreen->currentFrame->image, vk::ImageLayout::eColorAttachmentOptimal, &color, 1, &range);

			sScreen->currentFrame->transitionForPresent(buf);

			buf.endRecording();

			vk::CommandBuffer buff = buf;
			vk::PipelineStageFlags src = vk::PipelineStageFlagBits::eAllGraphics;
			vk::SubmitInfo submitInfo;
			submitInfo
				.commandBufferCount(1)
				.pCommandBuffers(&buff)
				.pWaitSemaphores(&sScreen->currentAcquireFrameSemaphore)
				.waitSemaphoreCount(1)
				.pWaitDstStageMask(&src)
				.signalSemaphoreCount(1)
				.pSignalSemaphores(&sScreen->currentFrame->renderingCompleteSemaphore);

			vk::queueSubmit(Context::mainQueue(), 1, &submitInfo, nullptr);

			sScreen->present();
			if (++frame > 10) {
				auto end = clock::now();
				std::chrono::duration<double> diff = end - begin;
				std::cout << "FPS: " << frame / diff.count();
				begin = clock::now();
				frame = 0;
			}
		}
		finalize();
	}

	void Application::quit()
	{
		PostQuitMessage(0);
	}

	void Application::finalize()
	{
		mGame->onExit();
		Context::destroy();
	}

	void nJinn::Application::doInitialize(APPLICATION_PARAMS)
	{
		shInstance = hInstance;
		Config::create();
		Config::parseCommandLine(args);
		Context::create();
		sScreen = new Screen(1280, 720);
		run();
	}
}