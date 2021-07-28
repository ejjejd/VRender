#pragma once
#include "vrender.h"
#include "vulkan/vulkan_app.h"

#include "graphics/shader.h"
#include "graphics/buffer.h"
#include "graphics/ubo.h"

#include "rendering/renderable.h"

namespace manager
{
	struct API RenderManager
	{
	private:
		VkRenderPass RenderPass;

		VkCommandPool CommandPool;
		std::vector<VkCommandBuffer> CommandBuffers;

		std::vector<VkFramebuffer> Framebuffers;

		VkSemaphore ImageAvailableSemaphore;
		VkSemaphore RenderFinishedSemaphore;

		vk::VulkanApp* VulkanApp;

		bool CreatePipeline();
		bool CreateRenderPass();
	public:
		graphics::UniformBuffer GlobalUBO;

		bool Setup(vk::VulkanApp& app);

		void Cleanup();

		bool Update(const std::vector<render::Renderable>& renderables);

		inline std::vector<VkFramebuffer> GetFBOs() const
		{
			return Framebuffers;
		}

		inline VkRenderPass GetRenderPass() const
		{
			return RenderPass;
		}
	};

}