#pragma once
#include "vrender.h"

#include "mesh.h"

namespace render
{
	struct Renderable
	{
		VkPipelineLayout GraphicsPipelineLayout;
		VkPipeline GraphicsPipeline;

		graphics::Buffer PositionsVertexBuffer;
	};

	inline void CleanupRenderable(const vk::VulkanApp& app, const Renderable& renderable)
	{
		vkDestroyPipelineLayout(app.Device, renderable.GraphicsPipelineLayout, nullptr);
		vkDestroyPipeline(app.Device, renderable.GraphicsPipeline, nullptr);
	}
}