#include "stdafx.hpp"
#include "DescriptorSet.hpp"

#include "Context.hpp"
#include "UniformBuffer.hpp"
#include "DescriptorAllocator.hpp"

namespace nJinn {
	DescriptorSet::DescriptorWriter::DescriptorWriter(const DescriptorSet & set) :
		mWrites(new vk::WriteDescriptorSet[set.mBindingCount]),
		mWriteInfos(new WriteInfo[set.mDescriptorCount]),
		mDescriptorSet(set.mDescriptorSet),
		mWriteCount(0),
		mDescriptorCount(0)
	{}

	DescriptorSet::DescriptorWriter::~DescriptorWriter()
	{
		context->dev().updateDescriptorSets(mWriteCount, mWrites, 0, nullptr);
		delete[] mWrites;
		delete[] mWriteInfos;
	}

	DescriptorSet::DescriptorWriter & DescriptorSet::DescriptorWriter::attachment(vk::DescriptorImageInfo * imageInfos, uint32_t binding, uint32_t count, uint32_t baseIndex)
	{
		for (uint32_t i = 0; i < count; ++i) {
			mWriteInfos[mDescriptorCount + i].image = imageInfos[i];
		}
		mWrites[mWriteCount]
			.setDescriptorCount(count)
			.setDescriptorType(vk::DescriptorType::eInputAttachment)
			.setDstArrayElement(baseIndex)
			.setDstSet(mDescriptorSet)
			.setDstBinding(binding)
			.setPImageInfo(&mWriteInfos[mDescriptorCount].image);
		mDescriptorCount += count;
		++mWriteCount;
		return *this;
	}

	DescriptorSet::DescriptorWriter & DescriptorSet::DescriptorWriter::uniformBuffer(class UniformBuffer * uniformBuffer, uint32_t binding, uint32_t count, uint32_t baseIndex)
	{
		for (uint32_t i = 0; i < count; ++i) {
			uniformBuffer[i].fillDescriptorInfo(mWriteInfos[mDescriptorCount + i].buffer);
		}
		mWrites[mWriteCount]
			.setDescriptorCount(count)
			.setDescriptorType(vk::DescriptorType::eUniformBufferDynamic)
			.setDstArrayElement(baseIndex)
			.setDstSet(mDescriptorSet)
			.setDstBinding(binding)
			.setPBufferInfo(&mWriteInfos[mDescriptorCount].buffer);
		mDescriptorCount += count;
		++mWriteCount;
		return *this;
	}

	DescriptorSet::DescriptorWriter & DescriptorSet::DescriptorWriter::uniformBuffer(vk::Buffer buffer, vk::DeviceSize offset, vk::DeviceSize size, uint32_t binding, uint32_t count, uint32_t baseIndex)
	{
		for (uint32_t i = 0; i < count; ++i) {
			mWriteInfos[mDescriptorCount + i].buffer
				.setBuffer(buffer)
				.setOffset(offset)
				.setRange(size);
		}
		mWrites[mWriteCount]
			.setDescriptorCount(count)
			.setDescriptorType(vk::DescriptorType::eUniformBufferDynamic)
			.setDstArrayElement(baseIndex)
			.setDstSet(mDescriptorSet)
			.setDstBinding(binding)
			.setPBufferInfo(&mWriteInfos[mDescriptorCount].buffer);
		mDescriptorCount += count;
		++mWriteCount;
		return *this;
	}

	DescriptorSet::DescriptorSet() :
		mBindingCount(0),
		mDescriptorCount(0)
	{}

	DescriptorSet::~DescriptorSet()
	{
		if(mDescriptorSet) context->dev().freeDescriptorSets(mParentPool, 1, &mDescriptorSet);
	}

}
