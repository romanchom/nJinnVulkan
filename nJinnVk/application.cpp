#include "stdafx.hpp"
#include "Application.hpp"

#include <chrono>

#include "Screen.hpp"
#include "Context.hpp"

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
		while (true) {
			if (sScreen->shouldClose()) break;
			sScreen->acquireFrame();
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
		Context::create();
		sScreen = new Screen(1280, 720);
		run();
	}
}