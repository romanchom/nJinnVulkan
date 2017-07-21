#pragma once

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#define MAIN_FUNCTION int APIENTRY wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, int nCmdShow)

#define APPLICATION_PARAMS_VAL lpCmdLine, hInstance
#define APPLICATION_PARAMS const wchar_t * args, HINSTANCE hInstance

#include <memory>

namespace nJinn {
	// TODO put it somwhere it belongs
	namespace os {
		extern HINSTANCE hInstance;
		extern int nCmdShow;
	}

	class GameBase {
	public:
		virtual ~GameBase() {};
		virtual void onInitialize() {}
		virtual void onUpdate() {}
		virtual void onPreRender() {}
		virtual void onRendered() {}
		virtual void onExit() {}
	};

	class Application
	{
	private:
		std::unique_ptr<GameBase> mGame;
		void gameLoop();
	public:
		Application(APPLICATION_PARAMS);
		~Application();
		template<typename T>
		int run();
		
		void quit();
	};

	template<typename T>
	inline int Application::run()
	{
		mGame = std::make_unique<T>();
		gameLoop();

		return 0;
	}
}
