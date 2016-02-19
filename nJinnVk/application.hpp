#pragma once

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#define MAIN_FUNCTION int APIENTRY wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, int nCmdShow)

#define APPLICATION_PARAMS_VAL lpCmdLine, hInstance
#define APPLICATION_PARAMS const wchar_t * args, HINSTANCE hInstance

namespace nJinn {
	class GameBase {
	public:
		virtual void onInitialize() {}
		virtual void onUpdate() {}
		virtual void onPreRender() {}
		virtual void onRendered() {}
		virtual void onExit() {}
	};

	class Application
	{
	public:
		template<typename T>
		static int initialize(APPLICATION_PARAMS);
		static void quit();
	private:
		static void run();
		static void finalize();
		Application() = delete;
		static void doInitialize(APPLICATION_PARAMS);
		static GameBase * mGame;
		static HINSTANCE shInstance;
		static int snCmdShow;
		static class Screen * sScreen;

		friend class Screen;
	};

	template<typename T>
	inline int Application::initialize(APPLICATION_PARAMS)
	{
		if (mGame) throw;
		mGame = new T();
		doInitialize(args, hInstance);
		return 0;
	}
}
