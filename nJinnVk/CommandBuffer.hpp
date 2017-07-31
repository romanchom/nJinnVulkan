#pragma once

#include <vulkan.hpp>

namespace nJinn {
	/// Manages the lifetime of vkCommandBuffer.
	/**
		This class contains command pool and a set of command buffers.
		They are kept in an internal queue and provided in round robin order.
	*/
	class CommandBuffer {
	public:
		/// Creates default command buffer ready to be used.
		CommandBuffer(uint32_t queueIndex = 0, vk::CommandBufferLevel level = vk::CommandBufferLevel::ePrimary);
		/// Immediatelly destroys command buffer and all owned resources.
		~CommandBuffer();

		/// Obtains next command buffer from queue and begins recording.
		inline void beginRecording();
		/// Ends recording of the command buffer.
		inline void endRecording();
		
		/// Gets current vkCommandBuffer that is ready for recording new commands.
		vk::CommandBuffer getRecordable() const noexcept { return mBuffers[mRecordableIndex]; }
		/// Gets current vkCommandBuffer that is ready to be executed.
		vk::CommandBuffer getExecutable() const noexcept { return mBuffers[mExecutableIndex]; }
		/// Gets current vkCommandBuffer that is ready to be executed.
		vk::CommandBuffer * operator->() {
			return mBuffers + mRecordableIndex;
		}
	private:
		enum { bufferCount = 2 };
		/// Command buffer pool from which command buffers are created.
		vk::CommandPool mPool;
		/// Queue of command buffers.
		vk::CommandBuffer mBuffers[bufferCount];
		/// Cached CommandBufferBeginInfo used every time beginRecording is called.
		vk::CommandBufferBeginInfo mBeginInfo;
		/// Index of the current executable command buffer in queue.
		uint32_t mExecutableIndex;
		/// Index of the current recordable command buffer in queue.
		uint32_t mRecordableIndex;
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
		++mRecordableIndex %= bufferCount;
		getRecordable().reset({});
		//getRecordable().reset(vk::CommandBufferResetFlagBits::eReleaseResources);
		getRecordable().begin(mBeginInfo);
	}
	
	/**
		Ends recording current command buffer.
		It is assumed that the command buffer is then submitted to the queue and executed once.
	*/
	inline void CommandBuffer::endRecording()
	{
		getRecordable().end();
		mExecutableIndex = mRecordableIndex;
	}
}