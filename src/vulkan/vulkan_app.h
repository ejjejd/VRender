#pragma once
#include "vrender.h"

namespace vk
{
	struct VulkanQueueFamilies
	{
		int32_t Graphics = -1;
		int32_t Compute = -1;
		int32_t Present = -1;
	};

	struct VulkanApp
	{
		GLFWwindow* GlfwWindow;

		VkInstance Instance;

		VkDebugUtilsMessengerEXT DebugMessenger;

		VkSurfaceKHR Surface;

		VkPhysicalDevice PhysicalDevice;
		VkDevice Device;

		VkPhysicalDeviceProperties DeviceProperties;

		VulkanQueueFamilies QueueFamilies;

		VkQueue GraphicsQueue;
		VkQueue ComputeQueue;
		VkQueue PresentQueue;

		VkCommandPool CommandPoolGQ;
		VkCommandPool CommandPoolCQ;

		VkSwapchainKHR SwapChain;

		VkImage DepthImage;
		VkDeviceMemory DepthImageMemory;
		VkImageView DepthImageView;

		VkFormat DepthFormat;

		VkFormat SwapChainFormat;
		VkExtent2D SwapChainExtent;

		std::vector<VkImage> SwapChainImages;
		std::vector<VkImageView> SwapChainImageViews;
	};

	bool API SetupVulkanApp(const uint16_t width, const uint16_t height, VulkanApp& app);
	void API CleanVulkanApp(VulkanApp& app);

	void API RunVulkanApp(VulkanApp& app, const std::function<void()>& callback);
}