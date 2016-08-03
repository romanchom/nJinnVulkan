#pragma once

#include <fstream>
#include "TeeStream.hpp"
#include "Console.hpp"

namespace nJinn {

	class Debug
	{
	public:
		enum VerbosityLevel{
			eNone,
			eError,
			eWarning,
			eInfo,
			eAll = eInfo,
		};
	private:
		std::ofstream logFileStream;
		Console outputConsole;
		TeeStream textOutput;
		int maximumVerbosityLevel;
	public:
		Debug(int initialVerbosity);
		template<typename T>
		void print(int verbosity, T t);

		template<typename T, typename... Args>
		void print(int verbosity, T t, Args... args);
		
		template<typename... Args>
		void log(Args... args);

		template<typename... Args>
		void error(Args... args);

		template<typename... Args>
		void warning(Args... args);

		void setVerbosity(int level);
	};

	template<typename T>
	inline void Debug::print(int verbosity, T t)
	{
		if (verbosity <= maximumVerbosityLevel) {
			textOutput << t;
		}
	}

	template<typename T, typename... Args>
	inline void Debug::print(int verbosity, T t, Args ... args)
	{
		print(verbosity, t);
		print(verbosity, args...);
	}

	template<typename... Args>
	inline void Debug::log(Args ...args)
	{
		print(VerbosityLevel::eInfo, args...);
	}
	template<typename ...Args>
	inline void Debug::error(Args ...args)
	{
		print(VerbosityLevel::eError, args...);
	}
	template<typename ...Args>
	inline void Debug::warning(Args ...args)
	{
		print(VerbosityLevel::eWarning, args...);
	}

	extern Debug * debug;
}
