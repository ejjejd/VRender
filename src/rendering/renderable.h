#pragma once
#include "vrender.h"

#include "mesh.h"
#include "graphics/descriptor.h"

namespace render
{
	struct Renderable
	{
		VkPipelineLayout GraphicsPipelineLayout;
		VkPipeline GraphicsPipeline;

		graphics::Buffer PositionsVertexBuffer;

		std::vector<graphics::Descriptor> Descriptors;
	};

	inline void CleanupRenderable(const vk::VulkanApp& app, const Renderable& renderable)
	{
		for (auto& d : renderable.Descriptors)
			vkDestroyDescriptorSetLayout(app.Device, d.DescriptorSetLayout, nullptr);

		vkDestroyPipelineLayout(app.Device, renderable.GraphicsPipelineLayout, nullptr);
		vkDestroyPipeline(app.Device, renderable.GraphicsPipeline, nullptr);
	}
}