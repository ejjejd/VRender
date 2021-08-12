#pragma once
#include "vrender.h"

#include "mesh.h"
#include "vulkan/descriptor.h"
#include "vulkan/ubo.h"

namespace render
{
	struct Renderable
	{
		VkPipelineLayout GraphicsPipelineLayout;
		VkPipeline GraphicsPipeline;

		std::vector<VkBuffer> Buffers;

		std::vector<vk::Descriptor> Descriptors;

		size_t PositionsCount;
	};

	inline void CleanupRenderable(const vk::VulkanApp& app, const Renderable& renderable)
	{
		for (auto& d : renderable.Descriptors)
			vk::CleanupDescriptor(app, d);
			
		vkDestroyPipelineLayout(app.Device, renderable.GraphicsPipelineLayout, nullptr);
		vkDestroyPipeline(app.Device, renderable.GraphicsPipeline, nullptr);
	}
}