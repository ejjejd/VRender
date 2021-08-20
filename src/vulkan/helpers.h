#pragma once
#include <optional>
#include <tuple>

#include "vrender.h"
#include "vulkan_app.h"
#include "vulkan/shader.h"

namespace vk
{
	struct Pipeline
	{
		VkPipelineLayout Layout;
		VkPipeline Handle;
	};


	std::optional<std::vector<char>> ReadShader(const std::string& filename);

	int32_t FindMemoryType(const vk::VulkanApp& app, uint32_t typeFilter, VkMemoryPropertyFlags properties);

	VkCommandBuffer BeginCommands(const VulkanApp& app, const VkCommandPool commandPool);
	void EndCommands(const VulkanApp& app, const VkCommandPool commandPool, 
					 const VkCommandBuffer commandBuffer, const VkQueue queue);


	void TransitionImageLayout(const VulkanApp& app, const VkImage image, 
							   const VkFormat format, const VkImageLayout oldLayout, const VkImageLayout newLayout);

	void CopyBufferToImage(const VulkanApp& app, const VkBuffer buffer, 
						   const VkImage image, const uint16_t width, const uint16_t height);


	std::optional<VkRenderPass> CreateRenderPass(const VulkanApp& app, const std::vector<VkAttachmentDescription>& attachments,
												 const std::vector<VkSubpassDescription>& subpasses,
												 const std::vector<VkSubpassDependency>& dependencies);

	std::optional<VkFramebuffer> CreateFramebuffer(const VulkanApp& app, const VkRenderPass& renderPass,
												   const std::vector<VkImageView>& attachments,
												   const uint16_t width, const uint16_t height);

	std::optional<Pipeline> CreateGraphicsPipeline(const VulkanApp& app,
												   const VkRenderPass& renderPass,
												   const vk::Shader& shader,
												   const std::vector< VkDescriptorSetLayout>& layouts,
												   const VkPipelineInputAssemblyStateCreateInfo& inputAssembly,
												   const VkPipelineViewportStateCreateInfo& viewportState,
												   const VkPipelineRasterizationStateCreateInfo& rasterizer,
												   const VkPipelineMultisampleStateCreateInfo& multisample,
												   const VkPipelineColorBlendStateCreateInfo& colorBlending,
												   const VkPipelineDepthStencilStateCreateInfo& depthState,
												   const VkPipelineDynamicStateCreateInfo& dynamicState);


	inline void CmdSetDepthOp(const VulkanApp& app, const VkCommandBuffer& cmdBuffer, 
							  const VkCompareOp value)
	{
		auto func = (PFN_vkCmdSetDepthCompareOpEXT)vkGetInstanceProcAddr(app.Instance, "vkCmdSetDepthCompareOpEXT");
		if (func != nullptr)
			return func(cmdBuffer, value);
	}
}												   
