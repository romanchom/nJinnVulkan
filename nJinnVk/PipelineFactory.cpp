#include "stdafx.hpp"
#include "PipelineFactory.hpp"


#include <boost/iostreams/device/mapped_file.hpp>

#include "Context.hpp"

namespace bi = boost::iostreams;

namespace nJinn {
	char * cacheUuidToFileName(const uint8_t * uuid) {
		static char filename[VK_UUID_SIZE * 2 + 5];
		for (int i = 0; i < VK_UUID_SIZE; ++i) {
			uint16_t chars;
			chars = uuid[i];
			chars = (chars & 0x0f) | ((chars & 0xf0) << 4);
			chars += (static_cast<uint16_t>('a') << 8) + 'a';
			reinterpret_cast<uint16_t &>(filename[i * 2]) = chars;
		}
		char * ext = ".vkc";
		strcpy(filename + VK_UUID_SIZE * 2, ext);
		return filename;
	}

	PipelineFactory::PipelineFactory()
	{
		vk::PhysicalDeviceProperties props;
		vk::getPhysicalDeviceProperties(Context::physDev(), &props);
		vk::PipelineCacheCreateInfo cacheInfo;
		try {
			bi::mapped_file_source file(cacheUuidToFileName(props.pipelineCacheUUID())); // TODO handle problems

			cacheInfo
				.initialDataSize(file.size())
				.pInitialData(file.data());
			dc(vk::createPipelineCache(Context::dev(), &cacheInfo, nullptr, &cache));
		} catch (...) {
			dc(vk::createPipelineCache(Context::dev(), &cacheInfo, nullptr, &cache));
		}
	}

	PipelineFactory::~PipelineFactory()
	{
		vk::PhysicalDeviceProperties props;
		vk::getPhysicalDeviceProperties(Context::physDev(), &props);
		size_t dataSize;
		dc(vk::getPipelineCacheData(Context::dev(), cache, &dataSize, nullptr));

		bi::mapped_file_params params;
		params.new_file_size = dataSize;
		params.length = dataSize;
		params.offset = 0;
		params.path = cacheUuidToFileName(props.pipelineCacheUUID());

		bi::mapped_file_sink file(params);
		dc(vk::getPipelineCacheData(Context::dev(), cache, &dataSize, file.data()));

		vk::destroyPipelineCache(Context::dev(), cache, nullptr);
	}

	vk::Pipeline PipelineFactory::createPipeline(Material & material, Mesh & mesh, vk::PipelineLayout layout, vk::RenderPass pass, uint32_t subpass,
		vk::PipelineRasterizationStateCreateInfo * rasterInfo, vk::PipelineDepthStencilStateCreateInfo * depthStencilInfo)
	{
		vk::PipelineViewportStateCreateInfo viewportState;
		viewportState
			.viewportCount(1)
			.scissorCount(1);

		vk::PipelineMultisampleStateCreateInfo multisampleState;
		vk::DynamicState dynamicStates[] = {
			vk::DynamicState::eViewport,
			vk::DynamicState::eScissor,
		};

		vk::PipelineDynamicStateCreateInfo dynamicState;
		dynamicState
			.pDynamicStates(dynamicStates)
			.dynamicStateCount(countof(dynamicStates));

		vk::GraphicsPipelineCreateInfo pipeInfo;
		material.fillPipelineInfo(pipeInfo);
		mesh.fillPipelineInfo(pipeInfo);
		pipeInfo
			.layout(layout)
			.renderPass(pass)
			.subpass(subpass)
			.pRasterizationState(rasterInfo)
			.pDepthStencilState(depthStencilInfo)
			.pViewportState(&viewportState)
			.pMultisampleState(&multisampleState)
			.pDynamicState(&dynamicState);

		vk::Pipeline ret;

		dc(vk::createGraphicsPipelines(Context::dev(), cache, 1, &pipeInfo, nullptr, &ret));

		return ret;
	}
}
