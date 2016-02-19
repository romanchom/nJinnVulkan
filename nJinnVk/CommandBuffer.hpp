#pragma once

#include <vulkan.hpp>

namespace nJinn {
	class CommandBuffer {
	public:
		CommandBuffer();
		~CommandBuffer();

		void beginRecording();

		void endRecording();
		
		operator vk::CommandBuffer() const { return buffer[currentIndex]; }
	private:
		vk::CommandPool pool;
		vk::CommandBuffer buffer[2];
		vk::CommandBufferBeginInfo beginInfo;
		size_t currentIndex;
	};

	inline void CommandBuffer::beginRecording() {
		++currentIndex %= 2;
		vk::beginCommandBuffer(*this, beginInfo);
	}
	
	inline void CommandBuffer::endRecording()
	{
		vk::endCommandBuffer(*this);
	}
}