#pragma once
#include "vrender.h"
#include "vulkan/vulkan_app.h"

#include "vulkan/shader.h"
#include "vulkan/compute_shader.h"
#include "vulkan/buffer.h"
#include "vulkan/ubo.h"
#include "vulkan/texture.h"
#include "vulkan/helpers.h"

#include "rendering/material.h"
#include "scene/scene_hi.h"
#include "rendering/camera.h"

#include "managers/asset_manager.h"

namespace manager
{
	constexpr uint8_t MaxPointLights = 32;
	constexpr uint8_t MaxSpotlights = 32;

	constexpr uint8_t  ShaderInputPositionLocation = 0;
	constexpr uint8_t  ShaderInputNormalLocation = 1;
	constexpr uint8_t  ShaderInputUvLocation = 2;
	constexpr uint8_t  ShaderInputTangentLocation = 3;
	constexpr uint8_t  ShaderInputBitangentLocation = 4;

	constexpr uint8_t ShaderDescriptorSetGlobalUBO = 0;
	constexpr uint8_t ShaderDescriptorSetMeshUBO = 1;
	constexpr uint8_t ShaderDescriptorSetMaterialUBO = 2;
	constexpr uint8_t ShaderDescriptorSetMaterialTextures = 3;

	constexpr uint8_t ShaderDescriptorBindCameraUBO = 0;
	constexpr uint8_t ShaderDescriptorBindLightUBO = 1;

	constexpr auto FromHdrToCubemapShader = "res/shaders/compute/generate_cubemap.spv";
	constexpr auto IrradianceMapComputeShader = "res/shaders/compute/generate_im.spv";

	class TextureManager
	{
	private:
		std::unordered_map<asset::AssetId, vk::Texture> TexturesLookup;

		vk::VulkanApp* App;
		AssetManager* AM;
	public:
		inline void Setup(vk::VulkanApp& app, AssetManager& am)
		{
			App = &app;
			AM = &am;
		}

		inline void Cleanup()
		{
			for (auto [id, t] : TexturesLookup)
				t.Cleanup();
		}

		vk::Texture GetOrCreate(const render::MaterialTexture& texture, const vk::DescriptorImageType type);

		inline void AddTexture(const asset::AssetId id, const vk::Texture& texture)
		{
			TexturesLookup[id] = texture;
		}
	};


	struct OffscreenRenderable
	{
		VkPipelineLayout PipelineLayout;
		VkPipeline Pipeline;

		vk::TextureDescriptor Descriptor;
	};

	struct OffscreenPass
	{
		VkRenderPass PassHandler;
		std::vector<VkFramebuffer> Framebuffers;

		vk::Texture ColorTexture;
		vk::Texture DepthTexture;

		OffscreenRenderable Renderable;
	};

	void CleanupOffscreenPass(const vk::VulkanApp& app, const OffscreenPass& pass);


	struct MeshRenderablesInfos
	{
		std::vector<VkPipelineLayout> GraphicsPipelineLayouts;
		std::vector<VkPipeline> GraphicsPipelines;

		std::vector<scene::RenderInfo> AdditionalInfo;

		std::vector<std::vector<vk::Buffer>> Buffers;

		std::vector<std::vector<vk::Descriptor>> Descriptors;

		std::vector<vk::UniformBuffer> MeshUBOs;
		std::vector<vk::UniformBuffer> MaterialUBOs;
	};

	void CleanupRenderablesInfos(const vk::VulkanApp& app, const MeshRenderablesInfos& infos);


	class API RenderManager
	{
	private:
		OffscreenPass HdrPass;

		VkRenderPass MainRenderPass;
		std::vector<VkFramebuffer> Framebuffers;

		std::vector<VkCommandBuffer> CommandBuffers;

		VkSemaphore ImageAvailableSemaphore;
		VkSemaphore RenderFinishedSemaphore;

		VkDescriptorPool DescriptorPool;
		VkDescriptorPool DescriptorPoolImage;
		VkDescriptorPool DescriptorPoolImageStorage;
	

		vk::UniformBuffer LightUBO;
		vk::UniformBuffer GlobalUBO;

		render::Camera ActiveCamera;

		MeshRenderablesInfos RenderablesInfos;

		TextureManager TM;

		manager::AssetManager* AM;
		vk::VulkanApp* VulkanApp;
	

		bool SetupRenderPassases();

		std::optional<vk::Pipeline> CreateMeshPipeline(vk::Shader& shader,
													   const std::vector<VkDescriptorSetLayout>& layouts);
		std::optional<vk::Pipeline> CreateMainPipeline(vk::Shader& shader,
													   const std::vector<VkDescriptorSetLayout>& layouts);

		std::vector<vk::Buffer> SetupMeshBuffers(const scene::MeshRenderable* mesh, vk::Shader& shader);
		std::vector<vk::Descriptor> SetupMeshDescriptors(const render::BaseMaterial& material, 
													     const vk::Shader& shader);

		void UpdateGlobalUBO();

		void Draw(const uint8_t imageId);
	public:
		void UpdateMeshUBO(const std::vector<scene::MeshRenderable*>& meshes);
		void UpdateLightUBO(const std::vector<scene::PointLight*>& pointLights,
							const std::vector<scene::Spotlight*>& spotlights);

		bool Setup(vk::VulkanApp& app, AssetManager& am);

		void Cleanup();

		void Update();

		void RegisterMesh(scene::MeshRenderable* mesh);

		inline void SetActiveCamera(const render::Camera& camera)
		{
			ActiveCamera = camera;
		}

		size_t GenerateCubemapFromHDR(const asset::AssetId id, const uint16_t resolution = 512);
		size_t GenerateIrradianceMap(const asset::AssetId id, const uint16_t resolution = 64);
	};

}