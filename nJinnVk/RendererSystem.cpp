#include "stdafx.hpp"
#include "RendererSystem.hpp"

#include "Context.hpp"
#include "PipelineFactory.hpp"
#include "Application.hpp"
#include "Screen.hpp"

namespace nJinn {
	void RendererSystem::createWorldDescriptorSet()
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

		dc(vk::createSampler(Context::dev(), &samplerInfo, nullptr, &immutableSamplers[0]));

		samplerInfo
			.anisotropyEnable(0)
			.maxAnisotropy(0.0f)
			.mipmapMode(vk::SamplerMipmapMode::eNearest)
			.compareEnable(1)
			.compareOp(vk::CompareOp::eLess); // TODO FIXME correct op for shadow mapping

		dc(vk::createSampler(Context::dev(), &samplerInfo, nullptr, &immutableSamplers[1]));

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

		dc(vk::createDescriptorSetLayout(Context::dev(), &worldDescInfo, nullptr, &descriptorSetLayouts[worldDescriptorSetIndex]));
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

		dc(vk::createDescriptorSetLayout(Context::dev(), &worldDescInfo, nullptr, &descriptorSetLayouts[objectDescriptorSetIndex]));
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

		dc(vk::createDescriptorSetLayout(Context::dev(), &worldDescInfo, nullptr, &descriptorSetLayouts[drawDescriptorSetIndex]));
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

		dc(vk::createPipelineLayout(Context::dev(), &layoutInfo, nullptr, &pipelineLayout));
	}
	void RendererSystem::destroyLayout()
	{
	}
	RendererSystem::RendererSystem() :
		someMaterial("shaders/triangle.vert.spv", "shaders/triangle.frag.spv", false)
	{
		createLayout();
		someMesh = Mesh::load("asteroid2.vbm");

		vk::PipelineRasterizationStateCreateInfo rast;
		vk::PipelineDepthStencilStateCreateInfo ds;
		ds
			.depthTestEnable(0)
			.stencilTestEnable(0)
			.maxDepthBounds(1)
			.depthCompareOp(vk::CompareOp::eAlways);

		somePipe = Context::pipeFact().createPipeline(someMaterial, *someMesh, pipelineLayout, Application::sScreen->renderPass, 0, &rast, &ds); 
	}

	RendererSystem::~RendererSystem()
	{
		vk::destroyPipelineLayout(Context::dev(), pipelineLayout, nullptr);
		for (size_t i = 0; i < descriptorSetCount; ++i) {
			vk::destroyDescriptorSetLayout(Context::dev(), descriptorSetLayouts[i], nullptr);
		}
		for (size_t i = 0; i < immutableSamplerCount; ++i) {
			vk::destroySampler(Context::dev(), immutableSamplers[i], nullptr);
		}
	}

	void RendererSystem::update(vk::CommandBuffer cmdBuf)
	{
		uint32_t asd[] = { 0, 0, 1280, 720 };
		vk::Rect2D rendArea;
		memcpy(&rendArea, asd, 16);

		float vals[] = { 0.3f, 0.3f, 0.0f, 1.0f };
		vk::ClearValue val;
		memcpy(&val, vals, 16);

		vk::Viewport view;
		view
			.height(720)
			.width(1280)
			.minDepth(0)
			.maxDepth(1)
			.x(0)
			.y(0);

		vk::RenderPassBeginInfo info;
		info
			.renderPass(Application::sScreen->renderPass)
			.framebuffer(Application::sScreen->currentFrame->frameBuffer)
			.renderArea(rendArea)
			.clearValueCount(1)
			.pClearValues(&val);

		vk::cmdBeginRenderPass(cmdBuf, &info, vk::SubpassContents::eInline);
		vk::cmdSetViewport(cmdBuf, 0, 1, &view);
		vk::cmdSetScissor(cmdBuf, 0, 1, &rendArea);
		vk::cmdBindPipeline(cmdBuf, vk::PipelineBindPoint::eGraphics, somePipe);
		someMesh->bindMesh(cmdBuf);
		someMesh->draw(cmdBuf);
		vk::cmdEndRenderPass(cmdBuf);
	}
}
