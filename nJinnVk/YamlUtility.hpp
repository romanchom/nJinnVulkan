#pragma once

#include <yaml-cpp/yaml.h>
#include <vulkan.hpp>
#include "Hash.hpp"

namespace YAML {
	using namespace nJinn::literals;
	
	template<>
	struct convert<vk::ShaderStageFlagBits> {
		typedef vk::ShaderStageFlagBits ss;

		static Node encode(const vk::ShaderStageFlagBits& rhs);
		static bool decode(const Node& node, vk::ShaderStageFlagBits& rhs);
	};

	template<>
	struct convert<vk::CompareOp> {
		typedef vk::CompareOp co;

		static Node encode(const vk::CompareOp& rhs);
		static bool decode(const Node& node, vk::CompareOp& rhs);
	};
}