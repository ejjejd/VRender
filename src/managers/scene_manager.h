#pragma once
#include "vrender.h"
#include "render_manager.h"

#include "graphics/shader.h"
#include "graphics/buffer.h"

#include "rendering/mesh.h"
#include "rendering/renderable.h"

namespace manager
{
	class API SceneManager
	{
	private:
		std::vector<std::reference_wrapper<render::Mesh>> RegisteredMeshes;
		std::vector<graphics::Buffer> MeshBuffers;

		std::vector<render::Renderable> Renderables;

		VkDescriptorPool DescriptorPool;

		RenderManager* RM;
		vk::VulkanApp* VulkanApp;

		std::vector<graphics::Descriptor> CreateDescriptors();

		bool CreatePipeline(const std::vector<VkDescriptorSetLayout>& layouts, 
							VkPipeline& pipeline, VkPipelineLayout& pipelineLayout, graphics::Shader& shader);
	public:
		void Setup(vk::VulkanApp& app, RenderManager& rm);

		void Cleanup();

		void RegisterMesh(render::Mesh& mesh);

		inline std::vector<render::Renderable> GetRenderables() const
		{
			return Renderables;
		}
	};
}