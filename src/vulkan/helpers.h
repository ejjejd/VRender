#pragma once
#include <optional>
#include <tuple>

#include "vrender.h"
#include "vulkan_app.h"
#include "vulkan/shader.h"
#include "vulkan/compute_shader.h"
#include "vulkan/descriptor.h"

namespace vk
{
	namespace layout
	{
		void SetImageLayoutFromUndefinedToTransfer(const vk::VulkanApp& app, const VkQueue queue,
												   const VkCommandPool commandPool, const VkImage image);
		void SetImageLayoutFromTransferToGraphicsShader(const vk::VulkanApp& app, const VkQueue queue,
														const VkCommandPool commandPool, const VkImage image);
		void SetImageLayoutFromTransferToComputeRead(const vk::VulkanApp& app, const VkQueue queue,
													 const VkCommandPool commandPool, const VkImage image);
		void SetCubeImageLayoutFromComputeWriteToGraphicsShader(const vk::VulkanApp& app, const VkQueue queue,
																const VkCommandPool commandPool, const VkImage image);
	}

	struct Pipeline
	{
		VkPipelineLayout Layout;
		VkPipeline Handle;
	};

	inline void DestoryPipeline(const vk::VulkanApp& app, const Pipeline& pipeline)
	{
		vkDestroyPipelineLayout(app.Device, pipeline.Layout, nullptr);
		vkDestroyPipeline(app.Device, pipeline.Handle, nullptr);
	}

	std::optional<std::vector<char>> ReadShader(const std::string& filename);

	int32_t FindMemoryType(const vk::VulkanApp& app, uint32_t typeFilter, VkMemoryPropertyFlags properties);

	VkCommandBuffer BeginCommands(const VulkanApp& app, const VkCommandPool commandPool);
	void EndCommands(const VulkanApp& app, const VkCommandPool commandPool, 
					 const VkCommandBuffer commandBuffer, const VkQueue queue);

	void CopyBufferToImage(const VulkanApp& app, const VkBuffer buffer, 
						   const VkImage image, const uint16_t width, const uint16_t height);


	std::optional<VkRenderPass> CreateRenderPass(const VulkanApp& app, const std::vector<VkAttachmentDescription>& attachments,
												 const std::vector<VkSubpassDescription>& subpasses,
												 const std::vector<VkSubpassDependency>& dependencies);

	std::optional<VkFramebuffer> CreateFramebuffer(const VulkanApp& app, const VkRenderPass& renderPass,
												   const std::vector<VkImageView>& attachments,
												   const uint16_t width, const uint16_t height);

	std::optional<Pipeline> CreateComputePipeline(const VulkanApp& app, 
												  const vk::ComputeShader& shader, 
												  const std::vector< VkDescriptorSetLayout>& layouts);


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

	void RunComputeShader(const VulkanApp& app, const vk::ComputeShader& cs, const Descriptor& descriptor, 
					      const uint16_t workGroupsX, const uint16_t workGroupsY, const uint16_t workGroupsZ);

	inline void CmdSetDepthOp(const VulkanApp& app, const VkCommandBuffer& cmdBuffer, 
							  const VkCompareOp value)
	{
		auto func = (PFN_vkCmdSetDepthCompareOpEXT)vkGetInstanceProcAddr(app.Instance, "vkCmdSetDepthCompareOpEXT");
		if (func != nullptr)
			return func(cmdBuffer, value);
	}

	inline void CmdSetCullMode(const VulkanApp& app, const VkCommandBuffer& cmdBuffer,
							   const VkCullModeFlags cullmode)
	{
		auto func = (PFN_vkCmdSetCullModeEXT)vkGetInstanceProcAddr(app.Instance, "vkCmdSetCullModeEXT");
		if (func != nullptr)
			return func(cmdBuffer, cullmode);
	}
}												   
