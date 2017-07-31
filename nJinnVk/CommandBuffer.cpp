#include "stdafx.hpp"
#include "CommandBuffer.hpp"

#include "Context.hpp"

namespace nJinn {
	CommandBuffer::CommandBuffer(uint32_t queueIndex, vk::CommandBufferLevel level) :
		mRecordableIndex(-1),
		mExecutableIndex(-1)
	{
		vk::CommandPoolCreateInfo poolInfo;
		poolInfo
			.setFlags(vk::CommandPoolCreateFlagBits::eResetCommandBuffer | vk::CommandPoolCreateFlagBits::eTransient)
			.setQueueFamilyIndex(queueIndex);
			
		mPool = context->dev().createCommandPool(poolInfo);

		vk::CommandBufferAllocateInfo bufferInfo;
		bufferInfo
			.setCommandBufferCount(bufferCount)
			.setCommandPool(mPool)
			.setLevel(level);

		context->dev().allocateCommandBuffers(&bufferInfo, mBuffers);

		mBeginInfo.setFlags(vk::CommandBufferUsageFlagBits::eOneTimeSubmit);
	}

	CommandBuffer::~CommandBuffer()
	{
		context->dev().destroyCommandPool(mPool);
	}
}