#pragma once
#include "vrender.h"

namespace vk
{
	struct VulkanQueueFamilies
	{
		int32_t vk;
		int32_t Present;
	};

	struct VulkanApp
	{
		GLFWwindow* GlfwWindow;

		VkInstance Instance;

		VkDebugUtilsMessengerEXT DebugMessenger;

		VkSurfaceKHR Surface;

		VkPhysicalDevice PhysicalDevice;
		VkDevice Device;

		VulkanQueueFamilies QueueFamilies;

		VkQueue GraphicsQueue;
		VkQueue PresentQueue;

		VkSwapchainKHR SwapChain;

		VkFormat SwapChainFormat;
		VkExtent2D SwapChainExtent;

		std::vector<VkImage> SwapChainImages;
		std::vector<VkImageView> SwapChainImageViews;
	};

	bool API SetupVulkanApp(const uint16_t width, const uint16_t height, VulkanApp& app);
	void API CleanVulkanApp(VulkanApp& app);

	void API RunVulkanApp(VulkanApp& app, const std::function<void()>& callback);
}