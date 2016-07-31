#include "stdafx.hpp"
#include "ResourceUploader.hpp"

#include "Context.hpp"

namespace nJinn {
	ResourceUploader * ResourceUploader::instance(nullptr);

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
				Context::dev().destroyBuffer(task.buffer);
				Context::dev().freeMemory(task.memory);
			}
			uploadTasks[i].clear();
		}
	}

	void ResourceUploader::addTask(const uploadTask & task)
	{
		uploadTasks[currentIndex].push_back(task);
		tasksAdded = true;
	}

	void ResourceUploader::create()
	{
		instance = new ResourceUploader();
	}

	void ResourceUploader::destroy()
	{
		delete instance;
	}

	void ResourceUploader::upload(const void * data, size_t size, vk::Buffer dst)
	{
		uploadTask task;

		vk::BufferCreateInfo bufferInfo;
		bufferInfo
			.setSize(size)
			.setUsage(vk::BufferUsageFlagBits::eTransferSrc);

		task.buffer = Context::dev().createBuffer(bufferInfo);

		vk::MemoryRequirements memReq = Context::dev().getBufferMemoryRequirements(task.buffer);

		vk::MemoryAllocateInfo allocInfo;
		allocInfo
			.setAllocationSize(memReq.size)
			.setMemoryTypeIndex(Context::uploadMemoryType());

		task.memory = Context::dev().allocateMemory(allocInfo);

		Context::dev().bindBufferMemory(task.buffer, task.memory, 0);

		void * pDest =Context::dev().mapMemory(task.memory, 0, VK_WHOLE_SIZE, vk::MemoryMapFlags());

		memcpy(pDest, data, size);

		if (!Context::isUploadMemoryTypeCoherent()) {
			Context::dev().flushMappedMemoryRanges(1, &vk::MappedMemoryRange(task.memory, 0, size));
		}
		vk::MemoryBarrier barr;
		vk::BufferMemoryBarrier preBarrier;

		instance->uploadCmdBuffer->copyBuffer(task.buffer, dst, 1, &vk::BufferCopy(0, 0, size));

		instance->addTask(task);
	}

	void ResourceUploader::doExecute()
	{
		vk::SubmitInfo submit;
		submit
			.setSignalSemaphoreCount(1)
			.setPSignalSemaphores(transfersCompleteSemaphore.get());

		if (tasksAdded) {
			uploadCmdBuffer.endRecording();

			vk::CommandBuffer buffer = uploadCmdBuffer;

			submit
				.setCommandBufferCount(1)
				.setPCommandBuffers(&buffer);

			++currentIndex %= 2;

			for (auto & task : uploadTasks[currentIndex]) {
				Context::dev().destroyBuffer(task.buffer);
				Context::dev().freeMemory(task.memory);
			}

			uploadTasks[currentIndex].clear();
			tasksAdded = false;

			uploadCmdBuffer.beginRecording();
		}

		Context::mainQueue().submit(1, &submit, nullptr);
	}
}
