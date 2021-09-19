#pragma once
#include "vrender.h"
#include "vulkan/vulkan_app.h"
#include "vulkan/shader.h"
#include "managers/asset_manager.h"
#include "vulkan/texture.h"

namespace render
{
	//Generate unique image ids for the images that will remain empty
	inline size_t GetDefaultImageId()
	{
		static size_t numberOfCalls = 0;
		return SIZE_MAX - numberOfCalls++;
	}

	struct MaterialTexture
	{
		utils::HashString Image;
		vk::TextureParams TextureParams;
		ImageChannels Channels;
	};

	inline vk::TextureParams CreateColorMapTextureParams()
	{
		vk::TextureParams params{};
		params.MagFilter = VK_FILTER_LINEAR;
		params.MinFilter = VK_FILTER_LINEAR;
		params.AddressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		params.AddressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;

		return params;
	}

	inline vk::TextureParams CreateInfoMapTextureParams()
	{
		vk::TextureParams params{};
		params.MagFilter = VK_FILTER_NEAREST;
		params.MinFilter = VK_FILTER_NEAREST;
		params.AddressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		params.AddressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;

		return params;
	}

	class BaseMaterial
	{
	public:
		virtual vk::Shader CreateShader(vk::VulkanApp& app) const = 0;
		virtual std::vector<MaterialTexture> GetMaterialTexturesIds() const = 0;
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
		MaterialTexture Albedo = { "", CreateColorMapTextureParams() };
		MaterialTexture Metallic = { "", CreateColorMapTextureParams() };
		MaterialTexture Roughness = { "", CreateColorMapTextureParams() };
		MaterialTexture Ao = { "", CreateColorMapTextureParams() };
		MaterialTexture Normal = { "", CreateColorMapTextureParams() };
		MaterialTexture IrradianceMap = { "", CreateColorMapTextureParams() };
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

		inline std::vector<MaterialTexture> GetMaterialTexturesIds() const override
		{
			return { Textures.Albedo, Textures.Metallic, 
					 Textures.Roughness, Textures.Ao,
			         Textures.Normal, Textures.IrradianceMap };
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

	class HdrMaterial : public BaseMaterial
	{
	public:
		MaterialTexture HdrTexture = { "", CreateColorMapTextureParams() };

		inline vk::Shader CreateShader(vk::VulkanApp& app) const override
		{
			vk::Shader shader;
			shader.Setup(app);

			shader.AddStage("res/shaders/other/hdr_map_vert.spv", VK_SHADER_STAGE_VERTEX_BIT);
			shader.AddStage("res/shaders/other/hdr_map_frag.spv", VK_SHADER_STAGE_FRAGMENT_BIT);

			return shader;
		}

		inline std::vector<MaterialTexture> GetMaterialTexturesIds() const override
		{
			return { HdrTexture };
		}

		inline  size_t GetMaterialInfoStride() const override
		{
			return 0;
		}

		inline void* GetMaterialData() const override
		{
			return nullptr;
		}
	};
}