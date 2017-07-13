#include "stdafx.hpp"
#include "Resource.hpp"
#include "Image.hpp"
#include "Context.hpp"

namespace nJinn {
	Image::Image() :
		mImage(nullptr),
		mDimension(0),
		mSizes{0, 0, 0},
		mMipLevels(0),
		mArraySize(0),
		mIsCube(false),
		mFormat(vk::Format::eUndefined),
		mDestroy(false)
	{

	}

	Image::~Image()
	{
		if (mDestroy) {
			context->dev().destroyImage(mImage);
		}
	}

	void Image::load(const std::string & resourceName)
	{}
}

