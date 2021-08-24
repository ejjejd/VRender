#pragma once
#include "vrender.h"
#include "vulkan/vulkan_app.h"

namespace vk
{
	class API ComputeShader
	{
	private:
		VkShaderModule Module;
		VkPipelineShaderStageCreateInfo ShaderStage;

		vk::VulkanApp* App;
	public:
		void Setup(vk::VulkanApp& app, const std::string& filepath);

		inline void Cleanup()
		{
			vkDestroyShaderModule(App->Device, Module, nullptr);
		}

		inline void ComputeShader::Dispatch(const VkCommandBuffer commandBuffer,
											const uint32_t workGroupsX,
											const uint32_t workGroupsY,
											const uint32_t workGroupsZ)
		{
			vkCmdDispatch(commandBuffer, workGroupsX, workGroupsY, workGroupsZ);
		}

		inline VkPipelineShaderStageCreateInfo GetStage() const
		{
			return ShaderStage;
		}
	};
}