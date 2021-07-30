#include "vulkan_app.h"

#include <set>
#include <fstream>

namespace vk
{
#ifdef NDEBUG
	constexpr bool EnableValidationLayers = false;
#else
	constexpr bool EnableValidationLayers = true;
#endif

	const std::vector<const char*> DesiredDeviceExtensions =
	{
		VK_KHR_SWAPCHAIN_EXTENSION_NAME
	};

	const std::vector<const char*> DesiredValidationLayers =
	{
		"VK_LAYER_KHRONOS_validation"
	};

	VKAPI_ATTR VkBool32 VKAPI_CALL DebugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
												 VkDebugUtilsMessageTypeFlagsEXT messageType,
												 const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
												 void* pUserData)
	{
		if (messageSeverity <= VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT)
			return VK_FALSE;

		LOG("Validation layer: %s\n", pCallbackData->pMessage)

		return VK_FALSE;
	}

	VkResult CreateDebugUtilsMessengerEXT(VkInstance instance,
										  const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
										  const VkAllocationCallbacks* pAllocator,
										  VkDebugUtilsMessengerEXT* pDebugMessenger)
	{
		auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
		if (func != nullptr)
			return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
		else
			return VK_ERROR_EXTENSION_NOT_PRESENT;
	}

	void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator)
	{
		auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
		if (func != nullptr)
			func(instance, debugMessenger, pAllocator);
	}

	bool CreateVulkanInstance(VulkanApp& app)
	{
		VkApplicationInfo appInfo{};
		appInfo.pApplicationName = "";
		appInfo.pEngineName = "";
		appInfo.apiVersion = VK_API_VERSION_1_0;
		appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
		appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
		appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
		appInfo.pNext = nullptr;

		uint32_t extensionsCount = 0;
		auto&& glfwExtensions = glfwGetRequiredInstanceExtensions(&extensionsCount);

		if (!glfwExtensions)
			return false;

		std::vector<const char*> desiredExtensions(glfwExtensions, glfwExtensions + extensionsCount);

		if constexpr (EnableValidationLayers)
			desiredExtensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);


		uint32_t availableExtensionsCount = 0;
		vkEnumerateInstanceExtensionProperties(nullptr, &availableExtensionsCount, nullptr);

		std::vector<VkExtensionProperties> availableExtensions(availableExtensionsCount);
		vkEnumerateInstanceExtensionProperties(nullptr, &availableExtensionsCount, &availableExtensions[0]);

		for (const auto& ext : desiredExtensions)
		{
			bool support = false;

			for (const auto& curExt : availableExtensions)
			{
				if (strcmp(ext, curExt.extensionName) == 0)
				{
					support = true;
					break;
				}
			}

			if (!support)
				return false;
		}


		uint32_t availableVLayersCount = 0;
		vkEnumerateInstanceLayerProperties(&availableVLayersCount, nullptr);

		std::vector<VkLayerProperties> availableVLayers(availableVLayersCount);
		vkEnumerateInstanceLayerProperties(&availableVLayersCount, &availableVLayers[0]);

		for (const auto& layer : DesiredValidationLayers)
		{
			bool support = false;

			for (const auto& curLayer : availableVLayers)
			{
				if (strcmp(layer, curLayer.layerName) == 0)
					support = true;
			}

			if (!support)
				return false;
		}

		VkInstanceCreateInfo instanceCreateInfo{};
		instanceCreateInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
		instanceCreateInfo.pApplicationInfo = &appInfo;
		instanceCreateInfo.enabledExtensionCount = desiredExtensions.size();
		instanceCreateInfo.ppEnabledExtensionNames = desiredExtensions.data();
		instanceCreateInfo.enabledLayerCount = DesiredValidationLayers.size();
		instanceCreateInfo.ppEnabledLayerNames = DesiredValidationLayers.data();

		if (vkCreateInstance(&instanceCreateInfo, nullptr, &app.Instance) != VK_SUCCESS)
			return false;

		if constexpr (EnableValidationLayers)
		{
			VkDebugUtilsMessengerCreateInfoEXT messengerCreateInfo{};
			messengerCreateInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
			messengerCreateInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT
												  | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT
												  | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
			messengerCreateInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT
											  | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT
											  | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
			messengerCreateInfo.pfnUserCallback = DebugCallback;
			messengerCreateInfo.pUserData = nullptr;

			if (CreateDebugUtilsMessengerEXT(app.Instance, &messengerCreateInfo, nullptr, &app.DebugMessenger) != VK_SUCCESS)
				return false;
		}

		return true;
	}

	VulkanQueueFamilies FindVulkanQueueFamilies(const VulkanApp& app, VkPhysicalDevice pd)
	{
		uint32_t queuesCount = 0;
		vkGetPhysicalDeviceQueueFamilyProperties(pd, &queuesCount, nullptr);

		std::vector<VkQueueFamilyProperties> properties(queuesCount);
		vkGetPhysicalDeviceQueueFamilyProperties(pd, &queuesCount, &properties[0]);

		VulkanQueueFamilies qf{ -1 };
		for (size_t i = 0; i < properties.size(); ++i)
		{
			if (properties[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)
				qf.vk = i;

			VkBool32 presentSupport = false;
			vkGetPhysicalDeviceSurfaceSupportKHR(pd, i, app.Surface, &presentSupport);
			if (presentSupport)
				qf.Present = i;

			if (qf.vk != -1 && qf.Present != -1)
				break;
		}

		return qf;
	}

	bool CheckDeviceExtensions(VkPhysicalDevice pd)
	{
		uint32_t extensionsCount = 0;
		vkEnumerateDeviceExtensionProperties(pd, nullptr, &extensionsCount, nullptr);

		std::vector<VkExtensionProperties> availableExtensions(extensionsCount);
		vkEnumerateDeviceExtensionProperties(pd, nullptr, &extensionsCount, &availableExtensions[0]);

		std::set<std::string> requiredExtensions(DesiredDeviceExtensions.begin(), DesiredDeviceExtensions.end());

		for (const auto& e : availableExtensions)
			requiredExtensions.erase(e.extensionName);

		return requiredExtensions.empty();
	}

	struct SwapChainDetails
	{
		VkSurfaceCapabilitiesKHR Capabilities;
		std::vector<VkSurfaceFormatKHR> Formats;
		std::vector<VkPresentModeKHR> PresentModes;
	};

	SwapChainDetails QuerySwapChainDetails(const VulkanApp& app, VkPhysicalDevice pd)
	{
		SwapChainDetails details;

		vkGetPhysicalDeviceSurfaceCapabilitiesKHR(pd, app.Surface, &details.Capabilities);


		uint32_t formatsCount = 0;
		vkGetPhysicalDeviceSurfaceFormatsKHR(pd, app.Surface, &formatsCount, nullptr);

		details.Formats.resize(formatsCount);
		vkGetPhysicalDeviceSurfaceFormatsKHR(pd, app.Surface, &formatsCount, &details.Formats[0]);


		uint32_t presentModesCount = 0;
		vkGetPhysicalDeviceSurfacePresentModesKHR(pd, app.Surface, &presentModesCount, nullptr);

		details.PresentModes.resize(presentModesCount);
		vkGetPhysicalDeviceSurfacePresentModesKHR(pd, app.Surface, &presentModesCount, &details.PresentModes[0]);

		return details;
	}

	bool IsDeviceSuitable(const VulkanApp& app, const VkPhysicalDevice& pd)
	{
		VkPhysicalDeviceProperties deviceProp;
		vkGetPhysicalDeviceProperties(pd, &deviceProp);

		auto details = QuerySwapChainDetails(app, pd);
		bool swapChainValid = !(details.PresentModes.empty() && details.Formats.empty());

		auto qf = FindVulkanQueueFamilies(app, pd);
		bool queueFamiliesValid = qf.vk != -1 & qf.Present != -1;

		return queueFamiliesValid
			&& CheckDeviceExtensions(pd)
			&& swapChainValid;
	}

	bool SetupDevice(VulkanApp& app)
	{
		uint32_t physicalDevicesCount = 0;
		vkEnumeratePhysicalDevices(app.Instance, &physicalDevicesCount, nullptr);

		if (physicalDevicesCount == 0)
			return false;

		std::vector<VkPhysicalDevice> physicalDevices(physicalDevicesCount);
		vkEnumeratePhysicalDevices(app.Instance, &physicalDevicesCount, &physicalDevices[0]);

		for (const auto& pd : physicalDevices)
		{
			if (IsDeviceSuitable(app, pd))
			{
				app.PhysicalDevice = pd;
				break;
			}
		}

		if (app.PhysicalDevice == VK_NULL_HANDLE)
			return false;

		app.QueueFamilies = FindVulkanQueueFamilies(app, app.PhysicalDevice);

		std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
		std::set<int32_t> queueFamiliesSet = { app.QueueFamilies.vk, app.QueueFamilies.Present };

		float queuePriority = 1.0f;

		for (auto q : queueFamiliesSet)
		{
			VkDeviceQueueCreateInfo queueCreateInfo{};
			queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
			queueCreateInfo.queueFamilyIndex = q;
			queueCreateInfo.queueCount = 1;
			queueCreateInfo.pQueuePriorities = &queuePriority;

			queueCreateInfos.push_back(queueCreateInfo);
		}

		VkPhysicalDeviceFeatures deviceFeatures{};

		VkDeviceCreateInfo deviceCreateInfo{};
		deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
		deviceCreateInfo.pQueueCreateInfos = queueCreateInfos.data();
		deviceCreateInfo.queueCreateInfoCount = queueCreateInfos.size();
		deviceCreateInfo.pEnabledFeatures = &deviceFeatures;
		deviceCreateInfo.ppEnabledExtensionNames = DesiredDeviceExtensions.data();
		deviceCreateInfo.enabledExtensionCount = DesiredDeviceExtensions.size();

		if (vkCreateDevice(app.PhysicalDevice, &deviceCreateInfo, nullptr, &app.Device) != VK_SUCCESS)
			return false;

		vkGetDeviceQueue(app.Device, app.QueueFamilies.vk, 0, &app.GraphicsQueue);
		vkGetDeviceQueue(app.Device, app.QueueFamilies.Present, 0, &app.PresentQueue);
	}

	VkSurfaceFormatKHR ChooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& formats)
	{
		for (const auto& f : formats)
		{
			if (f.format == VK_FORMAT_B8G8R8_SRGB
				&& f.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
			{
				return f;
			}
		}

		return formats[0];
	}

	VkPresentModeKHR ChooseSwapPresentMode(const std::vector<VkPresentModeKHR>& presentModes)
	{
		for (const auto& m : presentModes)
		{
			if (m == VK_PRESENT_MODE_MAILBOX_KHR)
				return m;
		}

		return VK_PRESENT_MODE_FIFO_KHR;
	}

	VkExtent2D ChooseSwapExtent(VulkanApp& app, const VkSurfaceCapabilitiesKHR& capabilities)
	{
		if (capabilities.currentExtent.width != UINT32_MAX)
			return capabilities.currentExtent;

		int width, height;
		glfwGetFramebufferSize(app.GlfwWindow, &width, &height);

		VkExtent2D extent{ 0 };
		extent.width = std::clamp((uint32_t)width, capabilities.minImageExtent.width, capabilities.maxImageExtent.height);
		extent.height = std::clamp((uint32_t)height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

		return extent;
	}

	bool CreateSwapChain(VulkanApp& app)
	{
		auto details = QuerySwapChainDetails(app, app.PhysicalDevice);

		auto surfaceFormat = ChooseSwapSurfaceFormat(details.Formats);

		auto presentMode = ChooseSwapPresentMode(details.PresentModes);

		auto extent = ChooseSwapExtent(app, details.Capabilities);

		uint32_t imageCount = details.Capabilities.maxImageCount;

		VkSwapchainCreateInfoKHR createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
		createInfo.surface = app.Surface;
		createInfo.minImageCount = imageCount;
		createInfo.imageFormat = surfaceFormat.format;
		createInfo.imageColorSpace = surfaceFormat.colorSpace;
		createInfo.imageExtent = extent;
		createInfo.imageArrayLayers = 1;
		createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

		auto families = app.QueueFamilies;
		uint32_t queueFamilyIndices[] = { families.vk, families.Present };

		if (families.vk != families.Present)
		{
			createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
			createInfo.queueFamilyIndexCount = 2;
			createInfo.pQueueFamilyIndices = queueFamilyIndices;
		}
		else
		{
			createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
			createInfo.queueFamilyIndexCount = 0;
			createInfo.pQueueFamilyIndices = nullptr;
		}

		createInfo.preTransform = details.Capabilities.currentTransform;
		createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
		createInfo.presentMode = presentMode;
		createInfo.clipped = VK_TRUE;
		createInfo.oldSwapchain = VK_NULL_HANDLE;

		if (vkCreateSwapchainKHR(app.Device, &createInfo, nullptr, &app.SwapChain) != VK_SUCCESS)
			return false;

		app.SwapChainFormat = surfaceFormat.format;
		app.SwapChainExtent = extent;

		vkGetSwapchainImagesKHR(app.Device, app.SwapChain, &imageCount, nullptr);
		app.SwapChainImages.resize(imageCount);
		vkGetSwapchainImagesKHR(app.Device, app.SwapChain, &imageCount, &app.SwapChainImages[0]);

		app.SwapChainImageViews.resize(imageCount);

		for (size_t i = 0; i < imageCount; ++i)
		{
			VkImageViewCreateInfo createInfo{};

			createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
			createInfo.image = app.SwapChainImages[i];
			createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
			createInfo.format = app.SwapChainFormat;

			createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
			createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
			createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
			createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;

			createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			createInfo.subresourceRange.baseMipLevel = 0;
			createInfo.subresourceRange.levelCount = 1;
			createInfo.subresourceRange.baseArrayLayer = 0;
			createInfo.subresourceRange.layerCount = 1;

			if (vkCreateImageView(app.Device, &createInfo, nullptr, &app.SwapChainImageViews[i]) != VK_SUCCESS)
				return false;
		}

		return true;
	}

	bool SetupVulkanApp(const uint16_t width, const uint16_t height, VulkanApp& app)
	{
		if (!glfwInit())
			return false;

		glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

		app.GlfwWindow = glfwCreateWindow(width, height, "Test", nullptr, nullptr);

		if (!app.GlfwWindow)
			return false;

		glfwSetInputMode(app.GlfwWindow, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

		if (!CreateVulkanInstance(app))
			return false;

		if (glfwCreateWindowSurface(app.Instance, app.GlfwWindow, nullptr, &app.Surface) != VK_SUCCESS)
			return false;

		if (!SetupDevice(app))
			return false;

		if (!CreateSwapChain(app))
			return false;
	}

	void CleanVulkanApp(VulkanApp& app)
	{
		for (size_t i = 0; i < app.SwapChainImageViews.size(); i++)
			vkDestroyImageView(app.Device, app.SwapChainImageViews[i], nullptr);

		vkDestroySwapchainKHR(app.Device, app.SwapChain, nullptr);

		vkDestroyDevice(app.Device, nullptr);

		vkDestroySurfaceKHR(app.Instance, app.Surface, nullptr);

		DestroyDebugUtilsMessengerEXT(app.Instance, app.DebugMessenger, nullptr);

		vkDestroyInstance(app.Instance, nullptr);

		glfwDestroyWindow(app.GlfwWindow);

		glfwTerminate();
	}

	void RunVulkanApp(VulkanApp& app, const std::function<void()>& callback)
	{
		while (!glfwWindowShouldClose(app.GlfwWindow))
		{
			glfwPollEvents();

			callback();
		}
	}
}