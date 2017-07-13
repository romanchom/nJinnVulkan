#include "stdafx.hpp"
#include "Camera.hpp"

#include "Screen.hpp"
#include "RendererSystem.hpp"
#include "Renderer.hpp"
#include "LightSource.hpp"

namespace nJinn {
	void Camera::draw(const std::unordered_set<class Renderer*> deferredObjects, const std::unordered_set<class LightSource*> lights)
	{
		vk::Rect2D rendArea;
		rendArea.extent.setWidth(screen->width()).setHeight(screen->height());

		vk::ClearValue vals[4];

		for (int i = 0; i < 4; ++i) {
			vals[i].color.setFloat32({ 0.1f, 0.1f, 0.1f, 0.1f });
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
		cmdbuf.beginRenderPass(info, vk::SubpassContents::eInline);
		cmdbuf.setViewport(0, 1, &view);
		cmdbuf.setScissor(0, 1, &rendArea);


		for (auto && obj : deferredObjects) {
			obj->draw(cmdbuf);
		}

		cmdbuf.nextSubpass(vk::SubpassContents::eInline);

		for (auto && light : lights) {
			light->draw(cmdbuf);
		}

		cmdbuf.endRenderPass();
		mCommandBuffer.endRecording();
	}

	Camera::Camera()
	{
		mGBuffer.initialize(screen->width(), screen->height());
		rendererSystem->registerCamera(this);
	}

	Camera::~Camera()
	{
		rendererSystem->unregisterCamera(this);
	}
}

