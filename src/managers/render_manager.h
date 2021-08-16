#pragma once
#include "vrender.h"
#include "vulkan/vulkan_app.h"

#include "vulkan/shader.h"
#include "vulkan/buffer.h"
#include "vulkan/ubo.h"

#include "rendering/renderable.h"
#include "rendering/light.h"

#include "graphics/camera.h"

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

	class TextureManager
	{
	private:
		std::unordered_map<asset::AssetId, graphics::Texture> TexturesLookup;

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

		graphics::Texture GetOrCreate(const render::MaterialTexture& texture);
	};

	class API RenderManager
	{
	private:
		VkRenderPass RenderPass;

		std::vector<VkCommandBuffer> CommandBuffers;

		std::vector<VkFramebuffer> Framebuffers;

		VkSemaphore ImageAvailableSemaphore;
		VkSemaphore RenderFinishedSemaphore;

		VkDescriptorPool DescriptorPool;
		VkDescriptorPool DescriptorPoolImage;

		std::vector<std::vector<vk::Buffer>> MeshBuffers;

		std::unordered_map<size_t, vk::UniformBuffer> MeshLookupUBOs;
		std::unordered_map<size_t, vk::UniformBuffer> MaterialLookupUBOs;

		std::vector<render::Renderable> Renderables;

		graphics::Camera ActiveCamera;

		vk::UniformBuffer LightUBO;
		vk::UniformBuffer GlobalUBO;

		TextureManager TM;

		vk::VulkanApp* VulkanApp;

		bool CreateRenderPass();

		std::vector<vk::Descriptor> CreateDescriptors(const render::BaseMaterial& material, const vk::Shader& shader,
													  const size_t meshId);
		void SetupBuffers(const render::Mesh& mesh, vk::Shader& shader, render::Renderable& renderable);

		bool CreatePipeline(const std::vector<VkDescriptorSetLayout>& layouts,
							VkPipeline& pipeline, VkPipelineLayout& pipelineLayout, vk::Shader& shader);

		void UpdateGlobalUBO();
	public:
		void UpdateMeshUBO(const std::vector<std::reference_wrapper<render::Mesh>>& meshes);
		void UpdateLightUBO(const std::vector<std::reference_wrapper<render::PointLight>>& pointLights,
							const std::vector<std::reference_wrapper<render::Spotlight>>& spotlights);

		bool Setup(vk::VulkanApp& app, AssetManager& am);

		void Cleanup();

		void Update();

		void RegisterMesh(const render::Mesh& mesh, const size_t meshId);

		inline void SetActiveCamera(const graphics::Camera& camera)
		{
			ActiveCamera = camera;
		}
	};

}