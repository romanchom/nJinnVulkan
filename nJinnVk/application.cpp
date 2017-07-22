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
#include "Context.hpp"
#include "Clock.hpp"



namespace nJinn {
	namespace os {
		HINSTANCE hInstance = 0;
		int nCmdShow = 1;
	}

	void Application::gameLoop()
	{
		mGame->onInitialize();
		double time = 0;
		uint64_t frames = 0;

		while (true) {
			if (screen->shouldClose()) break;
			clock->update();
			time += clock->delta();
			if (time > 1.0) {
				debug->log((clock->frame() - frames) / time, " FPS\n");
				frames = clock->frame();
				time = 0;
			}

			mGame->onUpdate();
			UniformBuffer::update();

			resourceUploader->execute();

			rendererSystem->update();
		}
	}

	Application::Application(APPLICATION_PARAMS)
	{
		os::hInstance = hInstance;
		config.parseCommandLine(args);
		config.parseDefaultConfigFile();
		nJinnStart();
	}

	Application::~Application()
	{
		mGame->onExit();
		context->mainQueue().waitIdle();
		GameObject::clearScene();
		nJinnStop();
	}

	void Application::quit()
	{
		PostQuitMessage(0);
	}
}