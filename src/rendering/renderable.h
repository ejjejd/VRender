#pragma once
#include "vrender.h"

#include "mesh.h"
#include "vulkan/descriptor.h"

namespace render
{
	struct Renderable
	{
		VkPipelineLayout GraphicsPipelineLayout;
		VkPipeline GraphicsPipeline;

		vk::Buffer PositionsVertexBuffer;

		std::vector<vk::Descriptor> Descriptors;
	};

	inline void CleanupRenderable(const vk::VulkanApp& app, const Renderable& renderable)
	{
		for (auto& d : renderable.Descriptors)
			vkDestroyDescriptorSetLayout(app.Device, d.DescriptorSetLayout, nullptr);

		vkDestroyPipelineLayout(app.Device, renderable.GraphicsPipelineLayout, nullptr);
		vkDestroyPipeline(app.Device, renderable.GraphicsPipeline, nullptr);
	}
}