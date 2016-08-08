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

	void Application::gameLoop()
	{
		mGame->onInitialize();
		while (true) {
			if (screen->shouldClose()) break;
			clock->update();
			screen->acquireFrame();
			mGame->onUpdate();
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
		}
	}

	Application::Application(APPLICATION_PARAMS)
	{
		os::hInstance = hInstance;
		config.parseCommandLine(args);
		config.parseDefaultConfigFile();
		systemStartup = std::make_unique<SystemStartup>();
	}

	Application::~Application()
	{
		mGame->onExit();
		context->mainQueue().waitIdle();
		GameObject::clearScene();
	}

	void Application::quit()
	{
		PostQuitMessage(0);
	}
}