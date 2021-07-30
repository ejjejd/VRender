#pragma once
#include "vrender.h"
#include "vulkan/vulkan_app.h"

namespace vk
{
	struct ShaderInput
	{
		std::vector<VkVertexInputBindingDescription> BindingDescriptions;
		std::vector<VkVertexInputAttributeDescription> AttributeDescriptions;
	};

	class API Shader
	{
	private:
		std::vector<VkShaderModule> ShaderModules;
		std::vector<VkPipelineShaderStageCreateInfo> Stages;

		ShaderInput Input;

		vk::VulkanApp* VulkanApp;
	public:
		inline void Setup(vk::VulkanApp& app)
		{
			VulkanApp = &app;
		}

		inline void Cleanup()
		{
			for (auto m : ShaderModules)
				vkDestroyShaderModule(VulkanApp->Device, m, nullptr);
		}

		void AddStage(const std::string filepath, const VkShaderStageFlagBits type);

		void AddInputBuffer(const VkFormat format, const uint8_t bind, const uint8_t location, 
							const size_t offset, const size_t stride, const VkVertexInputRate inputRate = VK_VERTEX_INPUT_RATE_VERTEX);

		inline void FetchStages(VkPipelineShaderStageCreateInfo* arr)
		{
			memcpy(arr, Stages.data(), sizeof(VkPipelineShaderStageCreateInfo) * Stages.size());
		}

		inline const std::vector<VkPipelineShaderStageCreateInfo>& GetStages() const
		{
			return Stages;
		}

		inline VkPipelineVertexInputStateCreateInfo GetInputState() const
		{
			VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
			vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
			vertexInputInfo.vertexBindingDescriptionCount = Input.BindingDescriptions.size();
			vertexInputInfo.pVertexBindingDescriptions = Input.BindingDescriptions.data();
			vertexInputInfo.vertexAttributeDescriptionCount = Input.AttributeDescriptions.size();
			vertexInputInfo.pVertexAttributeDescriptions = Input.AttributeDescriptions.data();

			return vertexInputInfo;
		}
	};
}