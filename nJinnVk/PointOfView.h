#pragma once

#include "Component.hpp"
#include "DescriptorSet.hpp"

namespace nJinn {
	class PointOfView : public Component {
	protected:
		DescriptorSet mDescriptorSet;

		virtual void update() = 0;
		//virtual void drwa(vk::CommandBuffer cmdbuf) = 0;
		
	public:
		PointOfView();
		virtual ~PointOfView();

		friend class RendererSystem;
	};
}