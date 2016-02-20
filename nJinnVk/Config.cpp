#include "stdafx.hpp"
#include "Config.hpp"

#include <fstream>

namespace nJinn {
	using namespace boost::program_options;

	options_description Config::optionsDescriptions;
	variables_map Config::variables;

	boost::program_options::options_description_easy_init Config::addOptions()
	{
		return optionsDescriptions.add_options();
	}

	void Config::create()
	{
		optionsDescriptions.add_options()
			("threads", value<uint32_t>()->default_value(1), "thread pool threads")
			("shadowMapRes", value<uint32_t>()->default_value(256), "shadow map resolution")
			("debugLevel", value<uint32_t>()->default_value(0), "additional runtime checks");

		std::ifstream configFile("config.cfg");
		if (configFile.good()) {
			store(parse_config_file(configFile, optionsDescriptions, false), variables);
		}
	}

	void Config::parseCommandLine(const wchar_t * line)
	{
		auto arguments = split_winmain(line);

		store(wcommand_line_parser(arguments).options(optionsDescriptions).run(), variables);

	}
}