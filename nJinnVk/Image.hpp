#pragma once

#include "Resource.hpp"

#include <vulkan.hpp>

namespace nJinn {
	class Image : public Resource {
	private:
		vk::Image mImage;
		uint32_t mDimension;
		uint32_t mSizes[3];
		uint32_t mMipLevels;
		uint32_t mArraySize;
		bool mIsCube;
		vk::Format mFormat;
		bool mDestroy;
	public:
		void fromRawHandle(vk::Image image);
		//void create();
		//void allocate();
		Image();
		virtual ~Image();
		virtual void load(const std::string & resourceName) override;

		void createView();
	};
}
