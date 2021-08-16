#pragma once
#include "vrender.h"
#include "render_manager.h"

#include "vulkan/shader.h"
#include "vulkan/buffer.h"
#include "vulkan/ubo.h"

#include "rendering/mesh.h"
#include "rendering/material.h"
#include "rendering/renderable.h"

#include "graphics/camera.h"
#include "graphics/light.h"
#include "graphics/texture.h"

namespace scene
{
	inline size_t IdsCounter = 0;

	class SceneObject
	{
	public:
		inline size_t GetId() const
		{
			return IdsCounter++;
		}
	};

	enum class RendererType
	{
		Mesh
	};

	class RendererObject : public SceneObject
	{
	protected:
		RendererType Type;
	public:
		inline RendererType GetRenderType() const
		{
			return Type;
		}
	};
}

namespace manager
{
	constexpr uint8_t MaxPointLights = 32;
	constexpr uint8_t MaxSpotlights = 32;

	/*class SceneManager
	{
	private:
		std::vector<std::reference_wrapper<render::Mesh>> RegisteredMeshes;
		std::vector<std::reference_wrapper<render::BaseMaterial>> RegisteredMaterials;

		std::vector<std::reference_wrapper<graphics::Camera>> Cameras;

		std::vector<std::reference_wrapper<graphics::PointLight>> RegisteredPointLights;
		std::vector<std::reference_wrapper<graphics::Spotlight>> RegisteredSpotlights;
	public:

	};*/

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

	class API SceneManager
	{
	private:
		VkRenderPass RenderPass;

		std::vector<VkCommandBuffer> CommandBuffers;

		std::vector<VkFramebuffer> Framebuffers;

		VkSemaphore ImageAvailableSemaphore;
		VkSemaphore RenderFinishedSemaphore;

		graphics::Camera ActiveCamera;

		std::vector<std::reference_wrapper<render::Mesh>> RegisteredMeshes;

		std::vector<std::reference_wrapper<graphics::PointLight>> RegisteredPointLights;
		std::vector<std::reference_wrapper<graphics::Spotlight>> RegisteredSpotlights;

		std::vector<std::reference_wrapper<graphics::Camera>> Cameras;

		std::vector<std::vector<vk::Buffer>> MeshBuffers;

		std::unordered_map<size_t, vk::UniformBuffer> MeshLookupUBOs;
		std::unordered_map<size_t, vk::UniformBuffer> MaterialLookupUBOs;

		std::vector<render::Renderable> Renderables;

		VkDescriptorPool DescriptorPool;
		VkDescriptorPool DescriptorPoolImage;

		vk::UniformBuffer LightUBO;
		vk::UniformBuffer GlobalUBO;

		size_t ActiveCameraId = 0;

		TextureManager TM;

		vk::VulkanApp* VulkanApp;

		bool CreateRenderPass();
		void UpdateUBO(const uint8_t imageId);

		std::vector<vk::Descriptor> CreateDescriptors(const render::BaseMaterial& material, const vk::Shader& shader);
		void SetupBuffers(render::Mesh& mesh, vk::Shader& shader, render::Renderable& renderable);

		bool CreatePipeline(const std::vector<VkDescriptorSetLayout>& layouts, 
							VkPipeline& pipeline, VkPipelineLayout& pipelineLayout, vk::Shader& shader);
	public:
		void Setup(vk::VulkanApp& app, RenderManager& rm, AssetManager& am);

		void Cleanup();

		void Update();

		void RegisterMesh(render::Mesh& mesh);

		inline void RegisterLight(graphics::PointLight& pl)
		{
			if(RegisteredPointLights.size() <= MaxPointLights)
				RegisteredPointLights.push_back(pl);
		}

		inline void RegisterLight(graphics::Spotlight& sl)
		{
			if (RegisteredSpotlights.size() <= MaxSpotlights)
				RegisteredSpotlights.push_back(sl);
		}

		inline size_t RegisterCamera(graphics::Camera& camera)
		{
			Cameras.push_back(camera);
			return Cameras.size();
		}

		inline void SetActiveCamera(const size_t cameraId)
		{
			if (cameraId >= Cameras.size())
			{
				LOG("Invalid camera selected!")
				return;
			}

			ActiveCameraId = cameraId;
		}

		inline std::vector<render::Renderable> GetRenderables() const
		{
			return Renderables;
		}
	};
}