#pragma once

#include <unordered_set>
#include <Eigen/Dense>
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
		template<typename T>
		using set_t = std::unordered_set<T *>;
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
	//private:
		struct GlobalUniformsStruct {
			Eigen::Matrix4f modelViewProjection;
		};
		set_t<class Renderer> mDeferredObjects;
		set_t<class LightSource> mLightSources;
		set_t<class Camera> mCameras;

		vk::RenderPass mDeferredRenderPass;
		
		UniformBuffer mGlobalUniforms;

		void createRenderPass();
	public:
		RendererSystem();
		~RendererSystem();

		void registerRenderer(class Renderer * renderer) { mDeferredObjects.emplace(renderer); }
		void unregisterRenderer(class Renderer * renderer) { mDeferredObjects.erase(renderer); }

		void registerLightSource(class LightSource * light) { mLightSources.emplace(light); }
		void unregisterLightSource(class LightSource * light) { mLightSources.erase(light); }

		void registerCamera(class Camera * camera) { mCameras.emplace(camera); }
		void unregisterCamera(class Camera * camera) { mCameras.erase(camera); }

		void update(vk::Semaphore * wSems, uint32_t wSemC, vk::Semaphore * sSems, uint32_t sSemsCw);

		vk::RenderPass renderPass() const { return mDeferredRenderPass; }
		
		vk::Sampler immutableSamplers[immutableSamplerCount];
	};

	extern RendererSystem * rendererSystem;
}