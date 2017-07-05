#pragma once

#include <vulkan.hpp>

namespace nJinn {
	class DescriptorSet {
	private:
		class DescriptorWriter {
		private:
			union WriteInfo {
				WriteInfo() {};
				vk::DescriptorImageInfo image;
				vk::DescriptorBufferInfo buffer;
				static_assert(sizeof(vk::DescriptorImageInfo) == sizeof(vk::DescriptorBufferInfo),
					"Union contains structs of different sizes. Cannot use arrays of union.");
			};
			vk::WriteDescriptorSet * mWrites;
			WriteInfo * mWriteInfos;
			vk::DescriptorSet mDescriptorSet;
			uint32_t mWriteCount;
			uint32_t mDescriptorCount;
		public:
			DescriptorWriter(const DescriptorSet & set);
			~DescriptorWriter();
			// TODO add some wrapper around images
			DescriptorWriter & attachment(vk::DescriptorImageInfo * imageInfos, uint32_t binding, uint32_t count = 1, uint32_t baseIndex = 0);
			DescriptorWriter & uniformBuffer(class UniformBuffer * uniformBuffer, uint32_t binding, uint32_t count = 1, uint32_t baseIndex = 0);
			DescriptorWriter & uniformBuffer(vk::Buffer buffer, vk::DeviceSize offset, vk::DeviceSize size, uint32_t binding, uint32_t count = 1, uint32_t baseIndex = 0);
		};

		vk::DescriptorSet mDescriptorSet;
		vk::DescriptorPool mParentPool;
		uint32_t mBindingCount;
		uint32_t mDescriptorCount;
	public:
		DescriptorSet();
		~DescriptorSet();
		DescriptorSet(const DescriptorSet &) = delete;
		DescriptorSet & operator=(const DescriptorSet &) = delete;

		DescriptorWriter write() { return DescriptorWriter(*this); }
		vk::DescriptorSet get() { return mDescriptorSet; }

		friend class DescriptorAllocator;
	};
}