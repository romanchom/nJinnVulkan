#include "stdafx.hpp"
#include "Camera.hpp"

#include "Screen.hpp"
#include "RendererSystem.hpp"
#include "Renderer.hpp"
#include "LightSource.hpp"
#include "GameObject.hpp"

namespace nJinn {
	struct Uniforms {
		Eigen::Matrix4f modelViewProjection;
	};

	void Camera::draw(const std::unordered_set<class Renderer*> deferredObjects, const std::unordered_set<class LightSource*> lights) {
		vk::Rect2D rendArea;
		rendArea.extent.setWidth(screen->width()).setHeight(screen->height());

		vk::ClearValue vals[4];

		for (int i = 0; i < 4; ++i) {
			vals[i].color.setFloat32({ 0.0f, 0.0f, 0.0f, 0.0f });
		}
		vals[0].depthStencil.setDepth(0.0f).setStencil(0);

		vk::Viewport view;
		view
			.setWidth((float)screen->width())
			.setHeight((float)screen->height())
			.setMinDepth(0)
			.setMaxDepth(1)
			.setX(0)
			.setY(0);

		vk::RenderPassBeginInfo info;
		info
			.setRenderPass(rendererSystem->mDeferredRenderPass)
			.setFramebuffer(mGBuffer.framebuffer())
			.setRenderArea(rendArea)
			.setClearValueCount(4)
			.setPClearValues(vals);

		mCommandBuffer.beginRecording();
		vk::CommandBuffer cmdbuf = mCommandBuffer.get();
		screen->transitionForDraw(mCommandBuffer);
		cmdbuf.beginRenderPass(info, vk::SubpassContents::eInline);
		cmdbuf.setViewport(0, 1, &view);

		auto descSet = mGeometryDescriptorSet.get();
		/*cmdbuf.bindDescriptorSets(vk::PipelineBindPoint::eGraphics,
			rendererSystem->mGeometryPipelineLayout, 0,
			1, &descSet,
			0, nullptr);*/

		Eigen::Matrix4d viewProjection = projection() * owner()->transform().inverse();
		auto layout = rendererSystem->mGeometryPipelineLayout;
		mUniformAllocator.nextCycle();
		for (auto && obj : deferredObjects) {
			auto allocation = mUniformAllocator.allocate(sizeof(Uniforms));
			auto data = reinterpret_cast<Uniforms *>(allocation.data);
			data->modelViewProjection = (viewProjection * obj->owner()->transform()).cast<float>();
			cmdbuf.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, layout, 0,
				1, &allocation.descriptorSet,
				1, &allocation.offset);
			obj->draw(cmdbuf);
		}

		cmdbuf.nextSubpass(vk::SubpassContents::eInline);

		descSet = mLightingDescriptorSet.get();
		cmdbuf.bindDescriptorSets(vk::PipelineBindPoint::eGraphics,
			rendererSystem->mLightingPipelineLayout, 0,
			1, &descSet,
			0, nullptr);

		for (auto && light : lights) {
			light->draw(cmdbuf);
		}
		cmdbuf.endRenderPass();

		screen->transitionForPresent(cmdbuf);
		mCommandBuffer.endRecording();
	}

	void Camera::computeProjectionMatrix() {
		double a, b;
		if (std::isinf(mFarClippingPlane)) {
			a = 0;
			b = mNearClippingPlane;
		}
		else {
			double farMinNear = mFarClippingPlane - mNearClippingPlane;
			a = mNearClippingPlane / farMinNear;
			b = mNearClippingPlane * mFarClippingPlane / farMinNear;
		}

		double scaleX = 1.0 / std::tan(mHorizontalFieldOfView * 0.5);
		double scaleY = 1.0 / std::tan(mVerticalFieldOfView * -0.5);

		mProjection <<
			scaleX, 0, 0, 0,
			0, 0, scaleY, 0,
			0, -a, 0, b,
			0, 1, 0, 0;

	}

	Camera::Camera() :
		mVerticalFieldOfView(1),
		mHorizontalFieldOfView(1),
		mNearClippingPlane(0.1),
		mFarClippingPlane(1),
		mProjectionDirty(true)
	{
		mProjection.setIdentity();

		rendererSystem->mGeometryDescriptorAllocator.allocateDescriptorSet(mGeometryDescriptorSet);
		rendererSystem->mLightingDescriptorAllocator.allocateDescriptorSet(mLightingDescriptorSet);
		
		mGBuffer.initialize(screen->width(), screen->height());
		mGBuffer.writeDescriptorSet(mLightingDescriptorSet);

		rendererSystem->registerCamera(this);
	}

	Camera::~Camera() {
		rendererSystem->unregisterCamera(this);
	}
}

