#include "stdafx.hpp"
#include "ResourceUploader.hpp"

#include "Context.hpp"

namespace nJinn {
	ResourceUploader * resourceUploader;

	ResourceUploader::ResourceUploader() :
		currentIndex(0),
		tasksAdded(false)
	{
		uploadCmdBuffer.beginRecording();
	}

	ResourceUploader::~ResourceUploader()
	{
		for (size_t i = 0; i < 2; ++i) {
			for (auto & task : uploadTasks[i]) {
				context->dev().destroyBuffer(task.buffer);
				context->dev().freeMemory(task.memory);
			}
			uploadTasks[i].clear();
		}
	}

	void ResourceUploader::addTask(const uploadTask & task)
	{
		uploadTasks[currentIndex].push_back(task);
		tasksAdded = true;
	}

	void ResourceUploader::upload(const void * data, size_t size, vk::Buffer dst)
	{
		uploadTask task;

		vk::BufferCreateInfo bufferInfo;
		bufferInfo
			.setSize(size)
			.setUsage(vk::BufferUsageFlagBits::eTransferSrc);

		task.buffer = context->dev().createBuffer(bufferInfo);

		vk::MemoryRequirements memReq = context->dev().getBufferMemoryRequirements(task.buffer);

		vk::MemoryAllocateInfo allocInfo;
		allocInfo
			.setAllocationSize(memReq.size)
			.setMemoryTypeIndex(context->uploadMemoryType());

		task.memory = context->dev().allocateMemory(allocInfo);

		context->dev().bindBufferMemory(task.buffer, task.memory, 0);

		void * pDest =context->dev().mapMemory(task.memory, 0, VK_WHOLE_SIZE, vk::MemoryMapFlags());

		memcpy(pDest, data, size);

		if (!context->isUploadMemoryTypeCoherent()) {
			auto range = vk::MappedMemoryRange(task.memory, 0, size);
			context->dev().flushMappedMemoryRanges(1, &range);
		}
		vk::MemoryBarrier barr;
		vk::BufferMemoryBarrier preBarrier;
		auto copy = vk::BufferCopy(0, 0, size);
		uploadCmdBuffer->copyBuffer(task.buffer, dst, 1, &copy);

		addTask(task);
	}

	void ResourceUploader::doExecute()
	{
		vk::SubmitInfo submit;
		submit
			.setSignalSemaphoreCount(1)
			.setPSignalSemaphores(transfersCompleteSemaphore.get());

		if (tasksAdded) {
			uploadCmdBuffer.endRecording();

			vk::CommandBuffer buffer = uploadCmdBuffer.get();

			submit
				.setCommandBufferCount(1)
				.setPCommandBuffers(&buffer);

			++currentIndex %= 2;

			for (auto & task : uploadTasks[currentIndex]) {
				context->dev().destroyBuffer(task.buffer);
				context->dev().freeMemory(task.memory);
			}

			uploadTasks[currentIndex].clear();
			tasksAdded = false;

			uploadCmdBuffer.beginRecording();
		}

		context->mainQueue().submit(1, &submit, nullptr);
	}
}
