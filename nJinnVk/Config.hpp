#pragma once

#include <boost/program_options.hpp>
#include <string>

namespace nJinn {
	class Config {
	private:
		boost::program_options::options_description optionsDescriptions;
		boost::program_options::variables_map variables;
	public:
		Config();

		template<typename T>
		const T & getValue(const std::string & key);

		boost::program_options::options_description_easy_init addOptions();

		void parseDefaultConfigFile();

		void parseCommandLine(const wchar_t * line);
		void parseCommandLine(int argc, const char ** argv);
		void parseConfigFile(const std::string & fileName);
	};

	template<typename T>
	const T & Config::getValue(const std::string & key)
	{
		auto & var  = variables[key];
		if (var.empty()) throw std::runtime_error("Option not recognized");
		return var.as<T>();
	}

	extern Config config;
}