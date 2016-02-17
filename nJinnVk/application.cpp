#include "stdafx.hpp"
#include "application.hpp"

#include "window.hpp"

namespace nJinn {
	gameBase * application::mGame = nullptr;
	HINSTANCE application::shInstance = 0;
	int application::snCmdShow = 1;
	class window * application::sWindow = nullptr;

	void application::run()
	{
		mGame->OnInitialize();
		
		while (true) {
			if (!sWindow->run()) break;
		}
		finalize();
	}

	void application::quit()
	{
		PostQuitMessage(0);
	}

	void application::finalize()
	{
		mGame->OnExit();
		
	}

	void nJinn::application::doInitialize(APPLICATION_PARAMS)
	{
		shInstance = hInstance;
		sWindow = new window(1280, 720);
		run();
	}
}