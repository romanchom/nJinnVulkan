#pragma once

#include <vulkan.hpp>
#include "MemoryAllocation.hpp"
#include "DescriptorAllocator.hpp"
#include "DescriptorSet.hpp"

namespace nJinn {
	class GBuffer {
		enum {
			depthStencilAttachmentIndex = 0,
			gBufferDiffuseColorAttachmentIndex,
			gBufferNormalSpecularAttachmentIndex,
			hdrColorAttachmentIndex,
			renderPassAttachmentsCount,

			geometrySubpassColorAttachmentsCount = gBufferNormalSpecularAttachmentIndex,
		};
	private:
		vk::Image mGBufferImages[renderPassAttachmentsCount];
		vk::ImageView mImageViews[renderPassAttachmentsCount];
		vk::ImageView mDepthOnlyImageView;
		MemoryAllocation mGBufferMemory;
		vk::Framebuffer mFramebuffers[3];
		DescriptorAllocator mDescriptorAllocator;
		DescriptorSet mDescSet;
	public:
		GBuffer();
		void initialize(uint32_t width, uint32_t height);
		vk::Image colorBuffer() { return mGBufferImages[hdrColorAttachmentIndex]; }
		vk::Framebuffer framebuffer();
		~GBuffer();
	};
}

