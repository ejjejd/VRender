#pragma once
#include "vrender.h"
#include "vulkan/vulkan_app.h"

namespace manager
{
	class API RenderManager
	{
	private:
		VkRenderPass RenderPass;
		VkPipelineLayout PipelineLayout;

		VkPipeline GraphicsPipeline;

		std::vector<VkFramebuffer> Framebuffers;

		VkCommandPool CommandPool;

		std::vector<VkCommandBuffer> CommandBuffers;

		VkSemaphore ImageAvailableSemaphore;
		VkSemaphore RenderFinishedSemaphore;

		VkBuffer VertexBuffer;
		VkDeviceMemory VertexBufferMemory;

		bool CreatePipeline(vk::VulkanApp& app);
		bool CreateRenderPass(vk::VulkanApp& app);
	public:
		bool Setup(vk::VulkanApp& app);

		void Cleanup(vk::VulkanApp& app);

		bool Update(vk::VulkanApp& app);
	};

}