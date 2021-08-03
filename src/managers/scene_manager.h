#pragma once
#include "vrender.h"
#include "render_manager.h"

#include "vulkan/shader.h"
#include "vulkan/buffer.h"
#include "vulkan/ubo.h"

#include "rendering/mesh.h"
#include "rendering/renderable.h"

#include "graphics/camera.h"

namespace manager
{
	constexpr uint8_t  ShaderInputPositionLocation = 0;

	constexpr uint8_t ShaderDescriptorSetGlobalUBO = 0;
	constexpr uint8_t ShaderDescriptorSetMeshUBO = 1;
	constexpr uint8_t ShaderDescriptorSetsCount = 2;

	class API SceneManager
	{
	private:
		std::vector<std::reference_wrapper<render::Mesh>> RegisteredMeshes;
		std::vector<std::reference_wrapper<graphics::Camera>> Cameras;

		std::vector<vk::Buffer> MeshBuffers;
		std::unordered_map<size_t, vk::UniformBuffer> MeshLookupUBOs;

		std::vector<render::Renderable> Renderables;

		VkDescriptorPool DescriptorPool;

		size_t ActiveCameraId = 0;

		RenderManager* RM;
		vk::VulkanApp* VulkanApp;

		std::vector<vk::Descriptor> CreateDescriptors(const vk::Shader& shader);
		void SetupBuffers(render::Mesh& mesh, vk::Shader& shader, render::Renderable& renderable);

		bool CreatePipeline(const std::vector<VkDescriptorSetLayout>& layouts, 
							VkPipeline& pipeline, VkPipelineLayout& pipelineLayout, vk::Shader& shader);
	public:
		graphics::Camera MainCamera;

		void Setup(vk::VulkanApp& app, RenderManager& rm);

		void Cleanup();

		void Update();

		void RegisterMesh(render::Mesh& mesh);

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