#include "stdafx.hpp"
#include "Material.hpp"

#include "MaterialFamily.hpp"

namespace nJinn {

	void Material::bind(vk::CommandBuffer cmdbuf)
	{
		//vk::cmdBindDescriptorSets(cmdbuf, vk::PipelineBindPoint::eGraphics, mMaterialFamily->layout(), 1, 1, &mDescriptorSet, 0, nullptr);
	}
}
