#include "shader.h"

#include <fstream>

namespace vk
{
	bool ReadShader(const std::string& filename, std::vector<char>& bytecode) 
	{
		std::ifstream file(filename, std::ios::ate | std::ios::binary);

		if (!file.is_open())
			return false;

		size_t fileSize = (size_t)file.tellg();
		bytecode.resize(fileSize);

		file.seekg(0);
		file.read(bytecode.data(), fileSize);

		file.close();

		return true;
	}

	void Shader::AddStage(const std::string filepath, const VkShaderStageFlagBits type)
	{
		std::vector<char> shaderByteCode;
		if (!ReadShader(filepath, shaderByteCode))
		{	
			LOG("Couldn't read shader %s\n", filepath)
			return;
		}

		VkShaderModuleCreateInfo moduleCreateInfo{};
		moduleCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
		moduleCreateInfo.codeSize = shaderByteCode.size();
		moduleCreateInfo.pCode = reinterpret_cast<const uint32_t*>(shaderByteCode.data());

		ShaderModules.push_back(VkShaderModule{});

		if (vkCreateShaderModule(VulkanApp->Device, &moduleCreateInfo, nullptr, &ShaderModules[ShaderModules.size() - 1]) != VK_SUCCESS)
		{
			LOG("Couldn't create shader module %s\n", filepath)
			return;
		}

		VkPipelineShaderStageCreateInfo stageCreateInfo{};
		stageCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		stageCreateInfo.stage = type;
		stageCreateInfo.module = ShaderModules[ShaderModules.size() - 1];
		stageCreateInfo.pName = "main";

		Stages.push_back(stageCreateInfo);
	}

	void Shader::AddInputBuffer(const VkFormat format, const uint8_t bind, const uint8_t location, 
								const size_t offset, const size_t stride, const VkVertexInputRate inputRate)
	{
		VkVertexInputBindingDescription bindingDescription{};
		bindingDescription.binding = 0;
		bindingDescription.stride = stride;
		bindingDescription.inputRate = inputRate;

		VkVertexInputAttributeDescription attributeDescription{};
		attributeDescription.location = location;
		attributeDescription.binding = 0;
		attributeDescription.format = format;
		attributeDescription.offset = offset;

		Input.BindingDescriptions.push_back(bindingDescription);
		Input.AttributeDescriptions.push_back(attributeDescription);
	}
}