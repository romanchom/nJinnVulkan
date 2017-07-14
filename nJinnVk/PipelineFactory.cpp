#include "stdafx.hpp"
#include "PipelineFactory.hpp"


#include <boost/iostreams/device/mapped_file.hpp>

#include "Context.hpp"

namespace bi = boost::iostreams;

namespace nJinn {
	PipelineFactory * pipelineFactory(nullptr);

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
		vk::PhysicalDeviceProperties props = context->physDev().getProperties();
		vk::PipelineCacheCreateInfo cacheInfo;
		try {
			bi::mapped_file_source file(cacheUuidToFileName(props.pipelineCacheUUID)); // TODO handle problems

			cacheInfo
				.setInitialDataSize(file.size())
				.setPInitialData(file.data());
			cache = context->dev().createPipelineCache(cacheInfo);
		} catch (...) {
			cache = context->dev().createPipelineCache(cacheInfo);
		}
	}

	PipelineFactory::~PipelineFactory()
	{
		vk::PhysicalDeviceProperties props = context->physDev().getProperties();
		size_t dataSize;
		context->dev().getPipelineCacheData(cache, &dataSize, nullptr);

		bi::mapped_file_params params;
		params.new_file_size = dataSize;
		params.length = dataSize;
		params.offset = 0;
		params.path = cacheUuidToFileName(props.pipelineCacheUUID);

		bi::mapped_file_sink file(params);
		context->dev().getPipelineCacheData(cache, &dataSize, file.data());
		context->dev().destroyPipelineCache(cache);
	}

	vk::Pipeline PipelineFactory::createPipeline(MaterialFamily & material, Mesh & mesh, vk::RenderPass pass, uint32_t subpass)
	{
		vk::Rect2D scissorRectangle;
		// from specification:
		// Evaluation of (offset.x + extent.width) must not cause a signed integer addition overflow
		uint32_t limit = static_cast<uint32_t>(std::numeric_limits<int32_t>::max());
		scissorRectangle.extent
			.setHeight(limit)
			.setWidth(limit);
	
		vk::PipelineViewportStateCreateInfo viewportState;
		viewportState
			.setViewportCount(1)
			.setScissorCount(1)
			.setPScissors(&scissorRectangle);

		vk::PipelineMultisampleStateCreateInfo multisampleState;
		vk::DynamicState dynamicStates[] = {
			vk::DynamicState::eViewport,
		};

		vk::PipelineDynamicStateCreateInfo dynamicState;
		dynamicState
			.setPDynamicStates(dynamicStates)
			.setDynamicStateCount(countof(dynamicStates));

		

		vk::GraphicsPipelineCreateInfo pipeInfo;
		material.fillPipelineInfo(pipeInfo);
		mesh.fillPipelineInfo(pipeInfo);
		pipeInfo
			.setRenderPass(pass)
			.setSubpass(subpass)
			.setPViewportState(&viewportState)
			.setPMultisampleState(&multisampleState)
			.setPDynamicState(&dynamicState);

		vk::Pipeline ret = context->dev().createGraphicsPipeline(cache, pipeInfo);

		return ret;
	}
}
