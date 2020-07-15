#include <iostream>
#include <boost/program_options.hpp>
#include <boost/algorithm/string.hpp>

#include <MeshData.hpp>
#include <MeshLoader.hpp>
#include <TypeConverter.hpp>

using namespace boost::program_options;


int main(int argc, char* argv[])
{
	bool help;
	bool verbose;
	std::string input;
	std::string output;
	uint32_t cacheSize(16);
	bool acmr;
	std::string positionFormat;
	std::string normalFormat;
	std::string textureFormat;
	bool tangent;
	bool directx;
	bool swap;

	try {
		options_description desc("Usage");
		desc.add_options()
			("help,h", "produce help message")
			("verbose,v", "more text")
			("input,i", value<std::string>(&input), "input model in .obj format")
			("output,o", value<std::string>(&output), "output to file")
			("reorder,r", value<uint32_t>(&cacheSize)->default_value(0), "reorder triangles and indicies for faster GPU processing")
			("acmr,a", "compute average cache miss ratio")
			("position-format,p", value<std::string>(&positionFormat)->default_value("float"), "position data format")
			("normal-format,n", value<std::string>(&normalFormat)->default_value("float"), "normal data format")
			("texture-format,t", value<std::string>(&textureFormat)->default_value("float"), "texture data format")
			("tangent", "compute tangent vector")
			("swap,s", "swap y and z axes")
			("directx,d", "convert to directx coordinate systems");

		positional_options_description p;
		p.add("input", 1);

		variables_map params;
		store(command_line_parser(argc, argv).options(desc).positional(p).run(), params);
		notify(params);

		help = params.count("help") != 0;
		verbose = params.count("verbose") != 0;
		acmr = params.count("acmr") != 0;
		tangent = params.count("tangent") != 0;
		directx = params.count("directx") != 0;
		swap = params.count("swap") != 0;

		if (help) {
			std::cout << desc << std::endl;
			return 0;
		}

		if (input.empty()) throw std::runtime_error("No input file given.");

		meshLoader::MeshLoader loader;
		loader.loadObj(input, verbose);
		if (cacheSize > 0) loader.optimizeForCache(cacheSize, verbose);
		loader.setConverter(0, meshLoader::TypeConverter::newConverter(positionFormat));
		loader.setConverter(1, meshLoader::TypeConverter::newConverter(textureFormat));
		loader.setConverter(2, meshLoader::TypeConverter::newConverter(normalFormat));

		if (directx) loader.convertToDX();
		if (swap) loader.swapYZ();
		if (tangent) {
			loader.calculateTangent();
			loader.setConverter(3, meshLoader::TypeConverter::newConverter(normalFormat));
		}

		loader.save(output);
		return 0;
	} catch (std::exception & e) {
		std::cerr << e.what();
		return -1;
	}
}

