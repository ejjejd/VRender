#include "shader.h"

#include "helpers.h"

namespace vk
{
	bool Shader::UpdateReflectMap(const std::vector<char>& shaderBytecode, const VkShaderStageFlagBits shaderType)
	{
		SpvReflectShaderModule spvModule;
		auto res = spvReflectCreateShaderModule(shaderBytecode.size(), shaderBytecode.data(), &spvModule);
		if (res != SPV_REFLECT_RESULT_SUCCESS)
			return false;

		ShaderReflectInfo reflectInfo;

		{
			uint32_t objCount = 0;
			spvReflectEnumerateInputVariables(&spvModule, &objCount, nullptr);

			SpvReflectInterfaceVariable** variables = new SpvReflectInterfaceVariable*[objCount];
			spvReflectEnumerateInputVariables(&spvModule, &objCount, variables);

			for (size_t i = 0; i < objCount; ++i)
			{
				auto input = variables[i];

				ShaderReflectInput inputInfo;
				inputInfo.LocationId = input->location;
				inputInfo.Name = input->name;

				reflectInfo.Inputs.push_back(inputInfo);
			}

			delete[] variables;
		}

		{
			uint32_t objCount = 0;
			spvReflectEnumerateDescriptorSets(&spvModule, &objCount, nullptr);

			SpvReflectDescriptorSet** descriptors = new SpvReflectDescriptorSet*[objCount];
			spvReflectEnumerateDescriptorSets(&spvModule, &objCount, descriptors);

			for (size_t i = 0; i < objCount; ++i)
			{
				auto set = descriptors[i];

				ShaderReflectDescriptorSet setInfo;
				setInfo.SetId = set->set;

				for (size_t j = 0; j < set->binding_count; ++j)
				{
					ShaderReflectDescriptorBinding bindingInfo;
					bindingInfo.BindId = set->bindings[j]->binding;
					bindingInfo.Name = set->bindings[j]->name;

					setInfo.Bindings.push_back(bindingInfo);
				}

				reflectInfo.DescriptorSets.push_back(setInfo);
			}

			delete[] descriptors;
		}

		spvReflectDestroyShaderModule(&spvModule);


		ReflectMap[shaderType] = reflectInfo;

		return true;
	}

	void Shader::AddStage(const std::string& filepath, const VkShaderStageFlagBits type)
	{
		auto bytecode = ReadShader(filepath);
		if(!bytecode)
		{	
			LOG("Couldn't read shader %s\n", filepath)
			return;
		}

		VkShaderModuleCreateInfo moduleCreateInfo{};
		moduleCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
		moduleCreateInfo.codeSize = bytecode->size();
		moduleCreateInfo.pCode = reinterpret_cast<const uint32_t*>(bytecode->data());

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

		if(!UpdateReflectMap(*bytecode, type))
			LOG("Couldn't reflect shader: %s", filepath)
	}

	void Shader::AddInputBuffer(const VkFormat format, const uint8_t bind, const uint8_t location, 
								const size_t offset, const size_t stride, const VkVertexInputRate inputRate)
	{
		VkVertexInputBindingDescription bindingDescription{};
		bindingDescription.binding = bind;
		bindingDescription.stride = stride;
		bindingDescription.inputRate = inputRate;

		VkVertexInputAttributeDescription attributeDescription{};
		attributeDescription.location = location;
		attributeDescription.binding = bind;
		attributeDescription.format = format;
		attributeDescription.offset = offset;

		Input.BindingDescriptions.push_back(bindingDescription);
		Input.AttributeDescriptions.push_back(attributeDescription);
	}
}