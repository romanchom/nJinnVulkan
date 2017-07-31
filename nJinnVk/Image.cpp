#include "stdafx.hpp"

#include <stdexcept>
#include <cstring>
#include <boost/iostreams/device/mapped_file.hpp>

#include "Resource.hpp"
#include "ResourceUploader.hpp"
#include "Image.hpp"
#include "Context.hpp"

namespace nJinn {
	Image::Image() :
		mImage(nullptr),
		mType(vk::ImageType::e1D),
		mSizes{0, 0, 0},
		mMipLevels(0),
		mArraySize(0),
		mIsCube(false),
		mFormat(vk::Format::eUndefined)
	{

	}

	Image::~Image()
	{
		context->dev().destroyImage(mImage);
		
	}

	void Image::load(const std::string & resourceName) {
		enum {
			DDS_MAGIC = 0x20534444u,
		};
		boost::iostreams::mapped_file_source file(resourceName);
		auto data = file.data();
		if (*reinterpret_cast<const uint32_t *>(data) == DDS_MAGIC) {
			loadDDS(data);
		} else {
			throw std::runtime_error("Unsupported texture format");
		}
	}

	void Image::loadDDS(const char * data)
	{
		enum PixelFormatFlags {
			DDPF_ALPHAPIXELS = 1 << 0,
			DDPF_ALPHA = 1 << 1,
			DDPF_FOURCC = 1 << 2,
		};

		struct PixelFormat {
			uint32_t size;
			uint32_t flags;
			uint32_t fourCC;
			uint32_t rgbBitCount;
			uint32_t rBitMask;
			uint32_t gBitMask;
			uint32_t bBitMask;
			uint32_t aBitMask;
		};
		struct Header {
			uint32_t magic;
			uint32_t size;
			uint32_t flags;
			uint32_t height;
			uint32_t width;
			uint32_t pitchOrLinearSize;
			uint32_t depth;
			uint32_t mipMapCount;
			uint32_t reserved1[11];
			PixelFormat pixelFormat;
			uint32_t caps;
			uint32_t caps2;
			uint32_t caps3;
			uint32_t caps4;
			uint32_t reserved2;
		};
		struct ExtendedHeader {
			uint32_t dxgiFormat;
			uint32_t resourceDimension;
			uint32_t miscFlag;
			uint32_t arraySize;
			uint32_t miscFlags2;
		};
		auto & header = *reinterpret_cast<const Header *>(data);
		data += sizeof(Header);
		mType = vk::ImageType::e2D;
		mSizes[0] = header.width;
		mSizes[1] = header.height;
		mMipLevels = header.mipMapCount;
		mArraySize = 1;
		mIsCube = false;

		bool compressed;
		uint32_t blockSize;
		uint32_t rowPitch;

		if ((0 != (header.pixelFormat.flags & DDPF_FOURCC)) &&
			(0 == memcmp(&header.pixelFormat.fourCC, "DX10", 4))) {
			auto & extendedHeader = *reinterpret_cast<const ExtendedHeader *>(data);
			data += sizeof(ExtendedHeader);
			if (extendedHeader.dxgiFormat == 99) { // DXGI_FORMAT_BC7_UNORM_SRGB
				mFormat = vk::Format::eBc7SrgbBlock;
				blockSize = 16;
				compressed = true;
			} else {
				throw std::runtime_error("Too lazy to implement other formats.");
			}
		} else {
			throw std::runtime_error("Too lazy to implement other formats.");
		}

		uint32_t width = mSizes[0];
		uint32_t height = mSizes[1];
		uint32_t totalSize = 0;

		uint32_t queueIndicies[] = {
			context->mainQueueFamilyIndex(),
			context->transferQueueFamilyIndex()
		};

		vk::ImageCreateInfo createInfo;
		createInfo.extent
			.setWidth(width)
			.setHeight(height);
		createInfo
			.setFormat(mFormat)
			.setImageType(vk::ImageType::e2D)
			.setMipLevels(mMipLevels)
			.setUsage(vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eSampled)
			.setInitialLayout(vk::ImageLayout::eUndefined)
			.setArrayLayers(1)
			.setSharingMode(vk::SharingMode::eExclusive)
			.setTiling(vk::ImageTiling::eOptimal);

		mImage = context->dev().createImage(createInfo);

		auto memReq = context->dev().getImageMemoryRequirements(mImage);

		mAllocation = memory->local().alloc(memReq.size);

		context->dev().bindImageMemory(mImage, mAllocation.memory(), mAllocation.offset());


		for (;;) {
			uint32_t mipSize = ((width + 3) / 4) *
				((height + 3) / 4) *
				blockSize;
			totalSize += mipSize;
			
			if (width == 1 && height == 1) break;

			width = (width + 1) / 2;
			height = (height + 1) / 2;
		}

		StagingBuffer buffer(totalSize);
		memcpy(buffer.pointer(), data, totalSize);

		resourceUploader->uploadImage(std::move(buffer), *this);
	}
}

