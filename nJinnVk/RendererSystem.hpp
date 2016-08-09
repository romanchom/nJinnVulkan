#pragma once

#include <boost/unordered_set.hpp>
#include <vulkan.hpp>

#include "Mesh.hpp"
#include "MaterialFamily.hpp"
#include "CommandBuffer.hpp"
#include "MemoryAllocation.hpp"
#include "MaterialFamily.hpp"
#include "Mesh.hpp"
#include "UniformBuffer.hpp"

namespace nJinn {
	class RendererSystem {
	public:
		typedef boost::unordered_set<class Renderer *> set_t;
		enum {
			worldDescriptorSetIndex = 0,
			objectDescriptorSetIndex,
			drawDescriptorSetIndex,
			descriptorSetCount,
			immutableSamplerCount = 2,
			worldDescriptorSetBindingCount = 2,
			objectDescriptorSetBindingCount = 4,
			drawDescriptorSetBindingCount = 1,

			depthStencilAttachmentIndex = 0,
			gBufferDiffuseColorAttachmentIndex,
			gBufferNormalSpecularAttachmentIndex,
			hdrColorAttachmentIndex,
			renderPassAttachmentsCount,


			geometrySubpassColorAttachmentsCount = gBufferNormalSpecularAttachmentIndex,

			geometrySubpassIndex = 0,
			lightingSubpassIndex,
			subpassCount,
		};
	private:

		set_t mRenderersSet;
		vk::RenderPass mDeferredRenderPass;

		vk::Image mGBufferImages[renderPassAttachmentsCount];
		vk::ImageView mImageViews[renderPassAttachmentsCount];
		MemoryAllocation mGBufferMemory;

		vk::Framebuffer mFramebuffer;

		MaterialFamily::handle mat;
		Mesh::handle mesh;
		vk::Pipeline pipe;
		UniformBuffer uniforms;
		vk::DescriptorSet mDescSet;

		void createRenderPass();
		void createGBuffer();
		void createFramebuffer();
	public:
		RendererSystem();
		~RendererSystem();

		void registerRenderer(class Renderer * renderer) { mRenderersSet.emplace(renderer); }
		void unregisterRenderer(class Renderer * renderer) { mRenderersSet.erase(renderer); }

		void update(vk::Semaphore * wSems, uint32_t wSemC, vk::Semaphore * sSems, uint32_t sSemsCw);

		vk::Sampler immutableSamplers[immutableSamplerCount];

		CommandBuffer cmdbuf;
	};

	extern RendererSystem * rendererSystem;
}