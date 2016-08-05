#include "stdafx.hpp"
#include "RendererSystem.hpp"

#include "Context.hpp"
#include "PipelineFactory.hpp"
#include "Application.hpp"
#include "Screen.hpp"
#include "Renderer.hpp"

namespace nJinn {
	RendererSystem * rendererSystem;

	RendererSystem::RendererSystem()
	{
	}

	RendererSystem::~RendererSystem()
	{
	}

	void RendererSystem::update(vk::Semaphore * wSems, uint32_t wSemC, vk::Semaphore * sSems, uint32_t sSemsC)
	{
		cmdbuf.beginRecording();
		screen->transitionForDraw(cmdbuf);
		

		uint32_t asd[] = { 0, 0, screen->width(), screen->height() };
		vk::Rect2D rendArea;
		memcpy(&rendArea, asd, 16);

		float vals[] = { 0.1f, 0.1f, 0.0f, 1.0f };
		vk::ClearValue val;
		memcpy(&val, vals, 16);

		vk::Viewport view;
		view
			.setWidth((float) screen->width())
			.setHeight((float) screen->height())
			.setMinDepth(0)
			.setMaxDepth(1)
			.setX(0)
			.setY(0);

		vk::RenderPassBeginInfo info;
		info
			.setRenderPass(screen->renderPass())
			.setFramebuffer(screen->framebuffer())
			.setRenderArea(rendArea)
			.setClearValueCount(1)
			.setPClearValues(&val);

		cmdbuf->beginRenderPass(info, vk::SubpassContents::eInline);
		cmdbuf->setViewport(0, 1, &view);
		cmdbuf->setScissor(0, 1, &rendArea);

		// TODO do this on separate thread or something
		for (auto rend : mRenderersSet) {
			rend->update();
		}

		for (auto rend : mRenderersSet) {
			rend->draw(cmdbuf);
		}


		cmdbuf->endRenderPass();
		screen->transitionForPresent(cmdbuf);
		cmdbuf.endRecording();

		vk::PipelineStageFlags src[] = {
			vk::PipelineStageFlagBits::eAllCommands,
			vk::PipelineStageFlagBits::eAllCommands,
			vk::PipelineStageFlagBits::eAllCommands,
		};

		vk::SubmitInfo submitInfo;
		submitInfo
			.setCommandBufferCount(1)
			.setPCommandBuffers(cmdbuf.get())
			.setPWaitSemaphores(wSems)
			.setWaitSemaphoreCount(wSemC)
			.setPWaitDstStageMask(src)
			.setSignalSemaphoreCount(sSemsC)
			.setPSignalSemaphores(sSems);

		context->mainQueue().submit(1, &submitInfo, nullptr);
	}
}
