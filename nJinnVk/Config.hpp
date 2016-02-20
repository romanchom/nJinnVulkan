#pragma once

#include <boost/program_options.hpp>
#include <string>

namespace nJinn {
	class Config {
		static boost::program_options::options_description optionsDescriptions;
		static boost::program_options::variables_map variables;
	public:
		template<typename T>
		static const T & getValue(const std::string & key);

		static boost::program_options::options_description_easy_init addOptions();

		static void create();

		static void parseCommandLine(const wchar_t * line);
	};

	template<typename T>
	const T & Config::getValue(const std::string & key)
	{
		return variables[key].as<T>();
	}
}