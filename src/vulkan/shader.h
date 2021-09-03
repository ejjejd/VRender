#pragma once
#include "vrender.h"
#include "vulkan/vulkan_app.h"

#include "spirv_reflect.h"

namespace vk
{
	struct ShaderReflectInput
	{
		std::string Name;
		uint32_t LocationId;
	};

	enum class DescriptorImageType
	{
		NotImage,
		Image2d,
		Cubemap
	};

	inline DescriptorImageType FromSpvImageDimToDescriptorImageType(const SpvDim dim)
	{
		switch(dim)
		{
		case SpvDim2D:	return DescriptorImageType::Image2d;
		case SpvDimCube: return DescriptorImageType::Cubemap;
		default: return DescriptorImageType::NotImage;
		}
	}

	struct ShaderReflectDescriptorBinding
	{
		std::string Name;
		uint32_t BindId;
		DescriptorImageType ImageType;
	};

	struct ShaderReflectDescriptorSet
	{
		uint32_t SetId;
		std::vector<ShaderReflectDescriptorBinding> Bindings;
	};

	struct ShaderReflectInfo
	{
		std::vector<ShaderReflectInput> Inputs;
		std::vector<ShaderReflectDescriptorSet> DescriptorSets;
	};

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

		std::unordered_map<VkShaderStageFlagBits, ShaderReflectInfo> ReflectMap;

		vk::VulkanApp* VulkanApp;

		bool UpdateReflectMap(const std::vector<char>& shaderBytecode, const VkShaderStageFlagBits shaderType);
	public:
		inline void Setup(vk::VulkanApp& app)
		{
			VulkanApp = &app;
		}

		inline void Cleanup() const
		{
			for (auto m : ShaderModules)
				vkDestroyShaderModule(VulkanApp->Device, m, nullptr);
		}

		void AddStage(const std::string& filepath, const VkShaderStageFlagBits type);

		void AddInputBuffer(const VkFormat format, const uint8_t bind, const uint8_t location, 
							const size_t offset, const size_t stride, const VkVertexInputRate inputRate = VK_VERTEX_INPUT_RATE_VERTEX);

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

		inline auto GetReflectMap() const
		{
			return ReflectMap;
		}
	};
}