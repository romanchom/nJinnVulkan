#include "stdafx.hpp"
#include "CommandBuffer.hpp"

#include "Context.hpp"

namespace nJinn {
	nJinn::CommandBuffer::CommandBuffer() :
		currentIndex(-1)
	{
		vk::CommandPoolCreateInfo poolInfo;
		poolInfo
			.setFlags(vk::CommandPoolCreateFlagBits::eResetCommandBuffer | vk::CommandPoolCreateFlagBits::eTransient)
			.setQueueFamilyIndex(context->mainQueueFamilyIndex());
			
		pool = context->dev().createCommandPool(poolInfo);

		vk::CommandBufferAllocateInfo bufferInfo;
		bufferInfo
			.setCommandBufferCount(bufferCount)
			.setCommandPool(pool)
			.setLevel(vk::CommandBufferLevel::ePrimary);

		context->dev().allocateCommandBuffers(&bufferInfo, buffer);

		beginInfo.setFlags(vk::CommandBufferUsageFlagBits::eOneTimeSubmit);
	}

	CommandBuffer::~CommandBuffer()
	{
		context->dev().destroyCommandPool(pool);
	}
}