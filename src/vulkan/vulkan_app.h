#pragma once
#include "vrender.h"

namespace vk
{
	struct VulkanApp
	{
		GLFWwindow* GlfwWindow;

		VkInstance Instance;

		VkDebugUtilsMessengerEXT DebugMessenger;

		VkSurfaceKHR Surface;

		VkPhysicalDevice PhysicalDevice;
		VkDevice Device;

		VkQueue GraphicsQueue;
		VkQueue PresentQueue;

		VkSwapchainKHR SwapChain;

		VkFormat SwapChainFormat;
		VkExtent2D SwapChainExtent;

		std::vector<VkImage> SwapChainImages;
		std::vector<VkImageView> SwapChainImageViews;

		VkRenderPass RenderPass;
		VkPipelineLayout PipelineLayout;

		VkPipeline GraphicsPipeline;

		std::vector<VkFramebuffer> Framebuffers;

		VkCommandPool CommandPool;

		std::vector<VkCommandBuffer> CommandBuffers;

		VkSemaphore ImageAvailableSemaphore;
		VkSemaphore RenderFinishedSemaphore;
	};

	bool API SetupVulkanApp(const uint16_t width, const uint16_t height, VulkanApp& app);
	void API CleanVulkanApp(VulkanApp& app);

	void API RunVulkanApp(VulkanApp& app, const std::function<void()>& callback);
}