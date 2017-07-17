#pragma once

#include <vulkan.hpp>

namespace nJinn {
	/// Manages the lifetime of vkCommanBuffer.
	/**
		This class contains command pool and a set of command buffers.
		They are kept in an internal queue and provided in round robin order.
	*/
	class CommandBuffer {
	public:
		/// Creates default command buffer ready to be used.
		CommandBuffer();
		/// Immediatelly destroys command buffer and all owned resources.
		~CommandBuffer();

		/// Obtains next command buffer from queue and begins recording.
		inline void beginRecording();
		/// Ends recording of the command buffer.
		inline void endRecording();
		
		/// Gets current vkCommandBuffer.
		vk::CommandBuffer get() { return buffer[currentIndex]; }
		/// Gets current vkCommandBuffer.
		vk::CommandBuffer * operator->() {
			return buffer + currentIndex;
		}
	private:
		enum { bufferCount = 2 };
		/// Command buffer pool from which command buffers are created.
		vk::CommandPool pool;
		/// Queue of command buffers.
		vk::CommandBuffer buffer[bufferCount];
		/// Cached CommandBufferBeginInfo used every time beginRecording is called.
		vk::CommandBufferBeginInfo beginInfo;
		/// Index of the current command buffer in queue.
		size_t currentIndex;
	};

	/**
		Increases current command buffer index,
		wraps around to zero in case of overflow.
		Then resets the command buffer and immediatelly begins recording.
		It is assumed that this function is called at most as often
		as the command buffer is submitted for execution and that
		at most 2 previous submitions are being executed or pending execution.
		Failure to do so results in undefined behaviour.
	*/
	inline void CommandBuffer::beginRecording() {
		++currentIndex %= bufferCount; // TODO possibly centralize this
		get().reset(vk::CommandBufferResetFlagBits::eReleaseResources);
		get().begin(beginInfo);
	}
	
	/**
		Ends recording current command buffer.
		It is assumed that the command buffer is then submitted to the queue and executed once.
	*/
	inline void CommandBuffer::endRecording()
	{
		get().end();
	}
}