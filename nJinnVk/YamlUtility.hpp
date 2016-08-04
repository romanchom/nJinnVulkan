#pragma once

#include <yaml-cpp/yaml.h>
#include <vulkan.hpp>
#include "Hash.hpp"

namespace YAML {
	using namespace nJinn::literals;
	template<>
	struct convert<vk::ShaderStageFlagBits> {
		typedef vk::ShaderStageFlagBits ss;

		static Node encode(const vk::ShaderStageFlagBits& rhs) {
			Node node;
			const char * string;
			switch (uint32_t(rhs)) {
			case uint32_t(ss::eVertex): string = "vertex"; break;
			case uint32_t(ss::eTessellationControl): string = "tesselationControl"; break;
			case uint32_t(ss::eTessellationEvaluation): string = "tessellationEvaluation"; break;
			case uint32_t(ss::eGeometry): string = "geometry"; break;
			case uint32_t(ss::eFragment): string = "fragment"; break;
			case uint32_t(ss::eCompute): string = "compute"; break;
			}
			node = string;
			return node;
		}

		static bool decode(const Node& node, vk::ShaderStageFlagBits& rhs) {
			uint64_t hash = nJinn::hash(node.as<std::string>());
			switch (hash) {
			case "vertex"_hash: rhs = ss::eVertex; break;
			case "tesselationControl"_hash: rhs = ss::eTessellationControl; break;
			case "tessellationEvaluation"_hash: rhs = ss::eTessellationEvaluation; break;
			case "geometry"_hash: rhs = ss::eGeometry; break;
			case "fragment"_hash: rhs = ss::eFragment; break;
			case "compute"_hash: rhs = ss::eCompute; break;
			default: return false;
			}
			return true;
		}
	};
}