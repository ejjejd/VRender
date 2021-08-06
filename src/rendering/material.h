#pragma once
#include "vrender.h"
#include "vulkan/vulkan_app.h"
#include "vulkan/shader.h"

namespace render
{
	class BaseMaterial
	{
	public:
		virtual vk::Shader CreateShader(vk::VulkanApp& app) const = 0;
		virtual size_t GetMaterialInfoStride() const = 0;
		virtual void* GetMaterialData() const = 0;
	};

	struct alignas(16) PbrMaterialInfo
	{
		glm::vec3 Albedo;
		float Metallic;
		float Roughness;
		float Ao;
	};

	class PbrMaterial : public BaseMaterial
	{
	public:
		mutable PbrMaterialInfo Info;

		inline vk::Shader CreateShader(vk::VulkanApp& app) const override
		{
			vk::Shader shader;
			shader.Setup(app);

			shader.AddStage("res/shaders/pbr/pbr_vert.spv", VK_SHADER_STAGE_VERTEX_BIT);
			shader.AddStage("res/shaders/pbr/pbr_frag.spv", VK_SHADER_STAGE_FRAGMENT_BIT);

			return shader;
		}

		inline  size_t GetMaterialInfoStride() const override
		{
			return sizeof(Info);
		}

		inline void* GetMaterialData() const override
		{
			return &Info;
		}
	};
}