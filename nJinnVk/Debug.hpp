#pragma once

#include <fstream>
#include "TeeStream.hpp"
#include "Console.hpp"

namespace nJinn {

	class Debug
	{
	public:
		enum class VerbosityLevel : int {
			none,
			error,
			warning,
			info,
			all = info,
		};
	private:
		std::ofstream logFileStream;
		Console outputConsole;
		TeeStream textOutput;
		int maximumVerbosityLevel;
	public:
		Debug(VerbosityLevel initialVerbosity);
		template<typename T>
		void print(VerbosityLevel verbosity, T t);

		template<typename T, typename... Args>
		void print(VerbosityLevel verbosity, T t, Args... args);
		
		template<typename... Args>
		void log(Args... args);

		template<typename... Args>
		void error(Args... args);

		template<typename... Args>
		void warning(Args... args);

		void setVerbosity(VerbosityLevel level);
	};

	template<typename T>
	inline void Debug::print(VerbosityLevel verbosity, T t)
	{
		if (int(verbosity) <= maximumVerbosityLevel) {
			textOutput << t;
		}
	}

	template<typename T, typename... Args>
	inline void Debug::print(VerbosityLevel verbosity, T t, Args ... args)
	{
		print(verbosity, t);
		print(verbosity, args...);
	}

	template<typename... Args>
	inline void Debug::log(Args ...args)
	{
		print(VerbosityLevel::info, args...);
	}
	template<typename ...Args>
	inline void Debug::error(Args ...args)
	{
		print(VerbosityLevel::error, args...);
	}
	template<typename ...Args>
	inline void Debug::warning(Args ...args)
	{
		print(VerbosityLevel::warning, args...);
	}

	extern Debug * debug;
}
