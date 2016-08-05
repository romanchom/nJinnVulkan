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
#include "Debug.hpp"
#include "SystemStartup.hpp"



namespace nJinn {
	namespace os {
		HINSTANCE hInstance = 0;
		int nCmdShow = 1;
	}

	typedef std::chrono::high_resolution_clock clock;

	Screen * Application::screen = nullptr;

	void Application::gameLoop()
	{
		auto begin = clock::now();
		mGame->onInitialize();
		size_t frame = 0;
		while (true) {
			if (screen->shouldClose()) break;
			screen->acquireFrame();

			UniformBuffer::update();

			resourceUploader->execute();

			vk::Semaphore waitSemaphores[] = {
				screen->waitingSemaphore(),
				resourceUploader->semaphore()
			};
			vk::Semaphore signalSemaphores[] = {
				screen->renderCompleteSemaphore()
			};

			rendererSystem->update(waitSemaphores, 2, signalSemaphores, 1);

			screen->present();

			++frame;
			auto end = clock::now();
			std::chrono::duration<double> diff = end - begin;
			if (diff.count() > 0.2) {
				debug->log("FPS: ", frame / diff.count(), "\n");
				begin = clock::now();
				frame = 0;
			}
		}
	}

	Application::Application(APPLICATION_PARAMS)
	{
		os::hInstance = hInstance;
		config.parseCommandLine(args);
		config.parseDefaultConfigFile();
		systemStartup = std::make_unique<SystemStartup>();

		screen = new Screen(config.getValue<uint32_t>("rendering.width"), config.getValue<uint32_t>("rendering.height"));
		rendererSystem->setScreen(screen);
	}

	Application::~Application()
	{
		mGame->onExit();
		context->mainQueue().waitIdle();
		GameObject::clearScene();
		delete screen;
	}

	void Application::quit()
	{
		PostQuitMessage(0);
	}
}