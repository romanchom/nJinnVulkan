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

	template<>
	struct convert<vk::CompareOp> {
		typedef vk::CompareOp co;

		static Node encode(const vk::CompareOp& rhs) {
			Node node;
			const char * string;
			switch (uint32_t(rhs)) {
			case uint32_t(co::eAlways):			string = "always"; break;
			case uint32_t(co::eEqual):			string = "equal"; break;
			case uint32_t(co::eGreater):		string = "greater"; break;
			case uint32_t(co::eGreaterOrEqual): string = "greaterEqual"; break;
			case uint32_t(co::eLess):			string = "less"; break;
			case uint32_t(co::eLessOrEqual):	string = "lessEqual"; break;
			case uint32_t(co::eNever):			string = "never"; break;
			case uint32_t(co::eNotEqual):		string = "notEqual"; break;
			}
			node = string;
			return node;
		}

		static bool decode(const Node& node, vk::CompareOp& rhs) {
			std::string val = node.as<std::string>();
			uint64_t hash = nJinn::hash(val);
			switch (hash) {
			case "always"_hash: rhs = co::eAlways; break;
			case "equal"_hash: rhs = co::eEqual; break;
			case "greater"_hash: rhs = co::eGreater; break;
			case "greaterEqual"_hash: rhs = co::eGreaterOrEqual; break;
			case "less"_hash: rhs = co::eLess; break;
			case "lessEqual"_hash: rhs = co::eLess; break;
			case "never"_hash: rhs = co::eNever; break;
			case "notEqual"_hash: rhs = co::eNotEqual; break;
			default: return false;
			}
			return true;
		}
	};
}