#pragma once

#include <vulkan.hpp>

#include "Resource.hpp"
#include "Memory.hpp"


namespace nJinn {
	class Image : public Resource {
	private:
		MemoryAllocation mAllocation;
		vk::Image mImage;
		vk::ImageType mType;
		uint32_t mSizes[3];
		uint32_t mMipLevels;
		uint32_t mArraySize;
		bool mIsCube;
		vk::Format mFormat;
		
		void loadDDS(const char * data);
	public:
		Image();
		virtual ~Image();
		virtual void load(const std::string & resourceName) override;
		vk::ImageType imageType() const noexcept { return mType; }
		vk::Format format() const noexcept { return mFormat; }
		vk::Image image() const noexcept { return mImage; }
		uint32_t width() const noexcept { return mSizes[0]; }
		uint32_t height() const noexcept { return mSizes[1]; }
		uint32_t depth() const noexcept { return mSizes[2]; }
		uint32_t mipLevels() const noexcept { return mMipLevels; }
		
		void createView();
	};
}
