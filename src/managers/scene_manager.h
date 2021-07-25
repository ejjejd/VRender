#pragma once
#include "vrender.h"
#include "render_manager.h"

#include "graphics/shader.h"
#include "graphics/buffer.h"

#include "scene_manager.h"

namespace manager
{
	class API SceneManager
	{
	private:
		VkPipelineLayout PipelineLayout;

		VkPipeline GraphicsPipeline;

		VkCommandPool CommandPool;

		std::vector<VkCommandBuffer> CommandBuffers;

		std::unique_ptr<graphics::Buffer> VertexBuffer;

		RenderManager* RM;
		vk::VulkanApp* VulkanApp;

		bool CreatePipeline(graphics::Shader& shader);
	public:
		bool Setup(vk::VulkanApp& app, RenderManager& rm);

		void Cleanup(const vk::VulkanApp& app);

		inline std::vector<VkCommandBuffer> GetCommandBuffers() const
		{
			return CommandBuffers;
		}
	};
}