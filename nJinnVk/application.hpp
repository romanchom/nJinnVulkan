#pragma once

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#define MAIN_FUNCTION int APIENTRY wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine,int nCmdShow)

#define APPLICATION_PARAMS_VAL lpCmdLine, hInstance
#define APPLICATION_PARAMS const wchar_t * args, HINSTANCE hInstance

namespace nJinn {
	class gameBase {
	public:
		virtual void OnInitialize() {}
		virtual void OnUpdate() {}
		virtual void OnPreRender() {}
		virtual void OnRendered() {}
		virtual void OnExit() {}
	};

	class application
	{
	public:
		template<typename T>
		static int initialize(APPLICATION_PARAMS);
		static void quit();
	private:
		static void run();
		static void finalize();
		application() = delete;
		static void doInitialize(APPLICATION_PARAMS);
		static gameBase * mGame;
		static HINSTANCE shInstance;
		static int snCmdShow;
		static class window * sWindow;

		friend class window;
	};

	template<typename T>
	inline int application::initialize(APPLICATION_PARAMS)
	{
		if (mGame) throw;
		mGame = new T();
		doInitialize(args, hInstance);
		return 0;
	}
}
