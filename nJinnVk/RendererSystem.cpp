#include "stdafx.hpp"
#include "RendererSystem.hpp"

#include "Context.hpp"
#include "PipelineFactory.hpp"
#include "Application.hpp"
#include "Screen.hpp"
#include "Renderer.hpp"

namespace nJinn {
	RendererSystem * rendererSystem;
	/*void RendererSystem::createWorldDescriptorSet()
	{
		vk::SamplerCreateInfo samplerInfo;
		samplerInfo
			.addressModeU(vk::SamplerAddressMode::eRepeat)
			.addressModeV(vk::SamplerAddressMode::eRepeat)
			.addressModeW(vk::SamplerAddressMode::eRepeat)
			.anisotropyEnable(1)
			.maxAnisotropy(16.0f)
			.magFilter(vk::Filter::eLinear)
			.minFilter(vk::Filter::eLinear)
			.mipmapMode(vk::SamplerMipmapMode::eLinear);

		dc(vk::createSampler(context->dev(), &samplerInfo, nullptr, &immutableSamplers[0]));

		samplerInfo
			.anisotropyEnable(0)
			.maxAnisotropy(0.0f)
			.mipmapMode(vk::SamplerMipmapMode::eNearest)
			.compareEnable(1)
			.compareOp(vk::CompareOp::eLess); // TODO FIXME correct op for shadow mapping

		dc(vk::createSampler(context->dev(), &samplerInfo, nullptr, &immutableSamplers[1]));

		vk::DescriptorSetLayoutBinding bindings[worldDescriptorSetBindingCount];
		bindings[0]
			.binding(0)
			.descriptorCount(1)
			.descriptorType(vk::DescriptorType::eUniformBuffer)
			.stageFlags(vk::ShaderStageFlagBits::eAllGraphics);

		bindings[1]
			.binding(1)
			.descriptorCount(immutableSamplerCount)
			.descriptorType(vk::DescriptorType::eSampler)
			.stageFlags(vk::ShaderStageFlagBits::eFragment)
			.pImmutableSamplers(immutableSamplers);

		vk::DescriptorSetLayoutCreateInfo worldDescInfo;
		worldDescInfo
			.bindingCount(worldDescriptorSetBindingCount)
			.pBindings(bindings);

		dc(vk::createDescriptorSetLayout(context->dev(), &worldDescInfo, nullptr, &descriptorSetLayouts[worldDescriptorSetIndex]));
	}

	void RendererSystem::createObjectDescriptorSet()
	{
		vk::DescriptorSetLayoutBinding bindings[objectDescriptorSetBindingCount];
		
		bindings[0]
			.binding(0)
			.descriptorCount(2)
			.descriptorType(vk::DescriptorType::eSampledImage)
			.stageFlags(vk::ShaderStageFlagBits::eFragment);

		bindings[1]
			.binding(1)
			.descriptorCount(2)
			.descriptorType(vk::DescriptorType::eSampler)
			.stageFlags(vk::ShaderStageFlagBits::eAllGraphics);

		bindings[2]
			.binding(2)
			.descriptorCount(2)
			.descriptorType(vk::DescriptorType::eSampledImage)
			.stageFlags(vk::ShaderStageFlagBits::eAllGraphics);

		bindings[3]
			.binding(3)
			.descriptorCount(1)
			.descriptorType(vk::DescriptorType::eUniformBuffer)
			.stageFlags(vk::ShaderStageFlagBits::eAllGraphics);


		vk::DescriptorSetLayoutCreateInfo worldDescInfo;
		worldDescInfo
			.bindingCount(objectDescriptorSetBindingCount)
			.pBindings(bindings);

		dc(vk::createDescriptorSetLayout(context->dev(), &worldDescInfo, nullptr, &descriptorSetLayouts[objectDescriptorSetIndex]));
	}

	void RendererSystem::createDrawDescriptorSet()
	{
		vk::DescriptorSetLayoutBinding bindings[drawDescriptorSetBindingCount];

		bindings[0]
			.binding(0)
			.descriptorCount(1)
			.descriptorType(vk::DescriptorType::eUniformBuffer)
			.stageFlags(vk::ShaderStageFlagBits::eAllGraphics);

		vk::DescriptorSetLayoutCreateInfo worldDescInfo;
		worldDescInfo
			.bindingCount(drawDescriptorSetBindingCount)
			.pBindings(bindings);

		dc(vk::createDescriptorSetLayout(context->dev(), &worldDescInfo, nullptr, &descriptorSetLayouts[drawDescriptorSetIndex]));
	}

	void RendererSystem::createLayout()
	{
		createWorldDescriptorSet();
		createObjectDescriptorSet();
		createDrawDescriptorSet();

		vk::PipelineLayoutCreateInfo layoutInfo;
		layoutInfo
			.setLayoutCount(descriptorSetCount)
			.pSetLayouts(descriptorSetLayouts);

		dc(vk::createPipelineLayout(context->dev(), &layoutInfo, nullptr, &pipelineLayout));
	}
	void RendererSystem::destroyLayout()
	{
	}*/
	RendererSystem::RendererSystem()
	{
		/*vk::DescriptorPoolSize poolSizes[2];
		poolSizes[0]
			.type(vk::DescriptorType::eUniformBuffer)
			.descriptorCount(10);
		poolSizes[1]
			.type(vk::DescriptorType::eSampler)
			.descriptorCount(20);


		vk::DescriptorPoolCreateInfo poolInfo;
		poolInfo
			.maxSets(10000)
			.poolSizeCount(2)
			.pPoolSizes(poolSizes);

		dc(vk::createDescriptorPool(context->dev(), &poolInfo, nullptr, &descPool));

		vk::DescriptorSetAllocateInfo setInfo;
		setInfo
			.descriptorPool(descPool)
			.descriptorSetCount(1)
			.pSetLayouts(&descriptorSetLayouts[worldDescriptorSetIndex]);

		dc(vk::allocateDescriptorSets(context->dev(), &setInfo, &descSet));

		vk::BufferCreateInfo buffInfo;
		buffInfo
			.usage(vk::BufferUsageFlagBits::eUniformBuffer)
			.size(100);

		vk::createBuffer(context->dev(), &buffInfo, nullptr, &buff);

		vk::MemoryAllocateInfo allocInfo;
		allocInfo
			.allocationSize(1024)
			.memoryTypeIndex(context->uploadMemoryType());

		vk::allocateMemory(context->dev(), &allocInfo, nullptr, &memory);

		vk::bindBufferMemory(context->dev(), buff, memory, 0);

		float * data;

		vk::mapMemory(context->dev(), memory, 0, 100, vk::MemoryMapFlags(), (void **) &data);

		data[0] = 1;
		data[1] = -0.5f;

		vk::DescriptorBufferInfo descBuffInfo;
		descBuffInfo
			.buffer(buff)
			.offset(0)
			.range(100);

		vk::WriteDescriptorSet descWrite;
		descWrite
			.dstSet(descSet)
			.dstBinding(0)
			.dstArrayElement(0)
			.descriptorCount(1)
			.descriptorType(vk::DescriptorType::eUniformBuffer)
			.pBufferInfo(&descBuffInfo);

		vk::updateDescriptorSets(context->dev(), 1, &descWrite, 0, nullptr);*/
	}

	RendererSystem::~RendererSystem()
	{
		/*vk::destroyPipelineLayout(context->dev(), pipelineLayout, nullptr);
		for (size_t i = 0; i < descriptorSetCount; ++i) {
			vk::destroyDescriptorSetLayout(context->dev(), descriptorSetLayouts[i], nullptr);
		}
		for (size_t i = 0; i < immutableSamplerCount; ++i) {
			vk::destroySampler(context->dev(), immutableSamplers[i], nullptr);
		}*/
	}

	void RendererSystem::update(vk::Semaphore * wSems, size_t wSemC, vk::Semaphore * sSems, size_t sSemsC)
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
			.setWidth(screen->width())
			.setHeight(screen->height())
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


		for (auto rend : Renderer::sRenderers) {
			//rend->mForwardMaterial->family()->bind();
			rend->draw(cmdbuf);
		}

		/*
		vk::cmdBindPipeline(cmdbuf, vk::PipelineBindPoint::eGraphics, somePipe);
		vk::cmdBindDescriptorSets(cmdbuf, vk::PipelineBindPoint::eGraphics, pipelineLayout, 0, 1, &descSet, 0, nullptr);
		someMesh->bindMesh(cmdbuf);
		someMesh->draw(cmdbuf);
		*/

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
