#include "stdafx.hpp"
#include "Application.hpp"

#include <chrono>
#include <iostream>

#include "Config.hpp"
#include "Screen.hpp"
#include "Context.hpp"
#include "CommandBuffer.hpp"
#include "Mesh.hpp"
#include "ResourceUploader.hpp"
#include "Material.hpp"
#include "RendererSystem.hpp"

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

		RendererSystem * rend = new RendererSystem();

		while (true) {
			if (sScreen->shouldClose()) break;
			sScreen->acquireFrame();

			// update

			ResourceUploader::execute();

			buf.beginRecording();

			sScreen->currentFrame->transitionForDraw(buf);

			rend->update(buf);

			sScreen->currentFrame->transitionForPresent(buf);

			buf.endRecording();

			vk::CommandBuffer buff = buf;
			vk::PipelineStageFlags src = vk::PipelineStageFlagBits::eAllGraphics;
			vk::Semaphore waitSemaphores[] = {
				sScreen->currentAcquireFrameSemaphore,
				ResourceUploader::semaphore()
			};
			vk::SubmitInfo submitInfo;
			submitInfo
				.commandBufferCount(1)
				.pCommandBuffers(&buff)
				.pWaitSemaphores(waitSemaphores)
				.waitSemaphoreCount(countof(waitSemaphores))
				.pWaitDstStageMask(&src)
				.signalSemaphoreCount(1)
				.pSignalSemaphores(&sScreen->currentFrame->renderingCompleteSemaphore);

			vk::queueSubmit(Context::mainQueue(), 1, &submitInfo, nullptr);
			sScreen->present();

			++frame;
			auto end = clock::now();
			std::chrono::duration<double> diff = end - begin;
			if (diff.count() > 0.2) {
				std::cout << "FPS: " << frame / diff.count() << std::endl;
				begin = clock::now();
				frame = 0;
			}
		}
		delete rend;
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