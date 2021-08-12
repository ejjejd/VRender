#pragma once
#include "vrender.h"
#include "render_manager.h"

#include "vulkan/shader.h"
#include "vulkan/buffer.h"
#include "vulkan/ubo.h"

#include "rendering/mesh.h"
#include "rendering/renderable.h"

#include "graphics/camera.h"
#include "graphics/light.h"
#include "graphics/texture.h"

namespace manager
{
	constexpr uint8_t MaxPointLights = 32;
	constexpr uint8_t MaxSpotlights = 32;

	constexpr uint8_t  ShaderInputPositionLocation = 0;
	constexpr uint8_t  ShaderInputNormalLocation = 1;
	constexpr uint8_t  ShaderInputUvLocation = 2;

	constexpr uint8_t ShaderDescriptorSetGlobalUBO = 0;
	constexpr uint8_t ShaderDescriptorSetMeshUBO = 1;
	constexpr uint8_t ShaderDescriptorSetMaterialUBO = 2;

	constexpr uint8_t ShaderDescriptorBindCameraUBO = 0;
	constexpr uint8_t ShaderDescriptorBindLightUBO = 1;

	class API SceneManager
	{
	private:
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

		size_t ActiveCameraId = 0;

		RenderManager* RM;
		vk::VulkanApp* VulkanApp;

		graphics::Texture Texture;

		std::vector<vk::Descriptor> CreateDescriptors(const render::BaseMaterial& material, const vk::Shader& shader);
		void SetupBuffers(render::Mesh& mesh, vk::Shader& shader, render::Renderable& renderable);

		bool CreatePipeline(const std::vector<VkDescriptorSetLayout>& layouts, 
							VkPipeline& pipeline, VkPipelineLayout& pipelineLayout, vk::Shader& shader);
	public:
		void Setup(vk::VulkanApp& app, RenderManager& rm);

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