#pragma once
#include "vrender.h"
#include "vulkan/vulkan_app.h"

#include "graphics/shader.h"
#include "graphics/buffer.h"

namespace manager
{
	struct API RenderManager
	{
	private:
		VkRenderPass RenderPass;

		std::vector<VkFramebuffer> Framebuffers;

		VkSemaphore ImageAvailableSemaphore;
		VkSemaphore RenderFinishedSemaphore;

		bool CreatePipeline(vk::VulkanApp& app);
		bool CreateRenderPass(vk::VulkanApp& app);
	public:
		bool Setup(vk::VulkanApp& app);

		void Cleanup(vk::VulkanApp& app);

		bool Update(vk::VulkanApp& app, const std::vector<VkCommandBuffer>& commandBuffers);

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