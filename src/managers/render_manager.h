#pragma once
#include "vrender.h"
#include "vulkan/vulkan_app.h"

#include "vulkan/shader.h"
#include "vulkan/buffer.h"
#include "vulkan/ubo.h"

#include "rendering/renderable.h"

#include "graphics/camera.h"

namespace manager
{
	struct API RenderManager
	{
	private:
		VkRenderPass RenderPass;

		std::vector<VkCommandBuffer> CommandBuffers;

		std::vector<VkFramebuffer> Framebuffers;

		VkSemaphore ImageAvailableSemaphore;
		VkSemaphore RenderFinishedSemaphore;

		graphics::Camera ActiveCamera;

		vk::VulkanApp* VulkanApp;

		bool CreateRenderPass();
		void UpdateUBO(const uint8_t imageId);
	public:
		vk::UniformBuffer GlobalUBO;

		bool Setup(vk::VulkanApp& app);

		void Cleanup();

		bool Update(const std::vector<render::Renderable>& renderables);

		inline void SetActiveCamera(const graphics::Camera& camera)
		{
			ActiveCamera = camera;
		}

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