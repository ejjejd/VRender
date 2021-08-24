#include "compute_shader.h"

#include "helpers.h"

namespace vk
{
	void ComputeShader::Setup(vk::VulkanApp& app, const std::string& filepath)
	{
		App = &app;

		auto bytecode = ReadShader(filepath);
		if (!bytecode)
		{
			LOGE("Couldn't read shader %s\n", filepath);
			return;
		}

	    VkShaderModuleCreateInfo moduleCreateInfo{};
		moduleCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
		moduleCreateInfo.codeSize = bytecode->size();
		moduleCreateInfo.pCode = reinterpret_cast<const uint32_t*>(bytecode->data());

		auto res = vkCreateShaderModule(app.Device, &moduleCreateInfo, nullptr, &Module);
		ASSERT(res == VK_SUCCESS, "Couldn't create shader module %s\n", filepath);


		ShaderStage = {};
		ShaderStage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		ShaderStage.stage = VK_SHADER_STAGE_COMPUTE_BIT;
		ShaderStage.module = Module;
		ShaderStage.pName = "main";
	}
}