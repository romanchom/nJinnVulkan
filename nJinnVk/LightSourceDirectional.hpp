#pragma once

#include "LightSource.hpp"
#include <vulkan.hpp>

namespace nJinn {
	class LightSourceDirectional : public LightSource
	{
	public:
		LightSourceDirectional();
		virtual ~LightSourceDirectional();
		virtual void draw(vk::CommandBuffer cmdbuf) override;
	};
}

