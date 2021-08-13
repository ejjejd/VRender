#pragma once
#include "vrender.h"
#include "vulkan/vulkan_app.h"
#include "vulkan/shader.h"
#include "managers/asset_manager.h"

namespace render
{
	class BaseMaterial
	{
	public:
		virtual vk::Shader CreateShader(vk::VulkanApp& app) const = 0;
		virtual std::vector<asset::AssetId> GetMaterialTexturesIds() const = 0;
		virtual size_t GetMaterialInfoStride() const = 0;
		virtual void* GetMaterialData() const = 0;
	};

	struct alignas(16) PbrMaterialParams
	{
		glm::vec3 Albedo = glm::vec3(1.0f);
		float Metallic = 1.0f;
		float Roughness = 1.0f;
		float Ao = 1.0f;
	};

	struct PbrMaterialTextures
	{
		asset::AssetId AlbedoId = -1;
		asset::AssetId MetallicId = -1;
		asset::AssetId RoughnessId = -1;
		asset::AssetId AoId = -1;
	};

	class PbrMaterial : public BaseMaterial
	{
	public:
		mutable PbrMaterialParams Params;

		PbrMaterialTextures Textures;

		inline vk::Shader CreateShader(vk::VulkanApp& app) const override
		{
			vk::Shader shader;
			shader.Setup(app);

			shader.AddStage("res/shaders/pbr/pbr_vert.spv", VK_SHADER_STAGE_VERTEX_BIT);
			shader.AddStage("res/shaders/pbr/pbr_frag.spv", VK_SHADER_STAGE_FRAGMENT_BIT);

			return shader;
		}

		inline std::vector<asset::AssetId> GetMaterialTexturesIds() const override
		{
			return { Textures.AlbedoId, Textures.MetallicId, Textures.RoughnessId, Textures.AoId };
		}

		inline  size_t GetMaterialInfoStride() const override
		{
			return sizeof(Params);
		}

		inline void* GetMaterialData() const override
		{
			return &Params;
		}
	};
}