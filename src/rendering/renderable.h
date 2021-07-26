#pragma once
#include "vrender.h"

#include "mesh.h"

namespace render
{
	struct Renderable
	{
		VkPipelineLayout GraphicsPipelineLayout;
		VkPipeline GraphicsPipeline;

		std::vector<VkCommandBuffer> CommandBuffers;
	};

	inline void CleanupRenderable(const vk::VulkanApp& app, const VkCommandPool& cp, const Renderable& renderable)
	{
		vkDestroyPipelineLayout(app.Device, renderable.GraphicsPipelineLayout, nullptr);
		vkDestroyPipeline(app.Device, renderable.GraphicsPipeline, nullptr);

		vkFreeCommandBuffers(app.Device, cp, renderable.CommandBuffers.size(), &renderable.CommandBuffers[0]);
	}
}