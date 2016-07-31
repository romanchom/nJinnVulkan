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
#include "Component.hpp"
#include "GameObject.hpp"
#include "UniformBuffer.hpp"

namespace nJinn {
	GameBase * Application::mGame = nullptr;
	HINSTANCE Application::shInstance = 0;
	int Application::snCmdShow = 1;
	Screen * Application::sScreen = nullptr;
	RendererSystem * Application::sRenderer = nullptr;

	typedef std::chrono::high_resolution_clock clock;

	void Application::run()
	{
		auto begin = clock::now();
		mGame->onInitialize();
		size_t frame = 0;
		sRenderer = new RendererSystem();
		while (true) {
			if (sScreen->shouldClose()) break;
			sScreen->acquireFrame();

			UniformBuffer::update();
			// update
			ComponentBase::updateComponents();

			ResourceUploader::execute();

			vk::Semaphore waitSemaphores[] = {
				sScreen->waitingSemaphore(),
				ResourceUploader::semaphore()
			};
			vk::Semaphore signalSemaphores[] = {
				sScreen->renderCompleteSemaphore()
			};
			
			sRenderer->update(waitSemaphores, 2, signalSemaphores, 1);
			
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
		finalize();
	}

	void Application::quit()
	{
		PostQuitMessage(0);
	}

	void Application::finalize()
	{
		mGame->onExit();
		Context::mainQueue().waitIdle();
		GameObject::clearScene();
		delete sRenderer;
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