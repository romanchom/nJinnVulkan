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
	private:
		Application() = delete;
		static void run();
		static void finalize();
		static void doInitialize(APPLICATION_PARAMS);
		static GameBase * mGame;
		static HINSTANCE shInstance;
		static int snCmdShow;
		static class Screen * sScreen;
		static class RendererSystem * sRenderer;

		friend class Screen;
	public:
		template<typename T>
		static int initialize(APPLICATION_PARAMS);
		static void quit();
		static Screen * screen(size_t index = 0) { return sScreen; }
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
