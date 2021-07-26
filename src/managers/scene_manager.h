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
		VkCommandPool CommandPool;

		RenderManager* RM;
		vk::VulkanApp* VulkanApp;
		
		std::vector<std::reference_wrapper<render::Mesh>> RegisteredMeshes;
		std::vector<graphics::Buffer> MeshBuffers;

		std::vector<render::Renderable> Renderables;

		bool CreatePipeline(VkPipeline& pipeline, VkPipelineLayout& pipelineLayout, graphics::Shader& shader);
		bool CreateCommandBuffers(const graphics::Buffer& buffer, const VkPipeline& pipeline, std::vector<VkCommandBuffer>& commandBuffers);
	public:
		inline void Setup(vk::VulkanApp& app, RenderManager& rm)
		{
			VulkanApp = &app;
			RM = &rm;
		}

		void Cleanup();

		void RegisterMesh(render::Mesh& mesh);

		inline std::vector<render::Renderable> GetRenderables() const
		{
			return Renderables;
		}
	};
}