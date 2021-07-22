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

	bool CreateInstance(VulkanApp& app)
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

	struct QueueFamilies
	{
		int32_t Graphics;
		int32_t Present;

		inline bool IsComplete() const
		{
			return Graphics != -1 && Present != -1;
		}
	};

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

	QueueFamilies FindQueueFamilies(const VulkanApp& app, VkPhysicalDevice pd)
	{
		uint32_t queuesCount = 0;
		vkGetPhysicalDeviceQueueFamilyProperties(pd, &queuesCount, nullptr);

		std::vector<VkQueueFamilyProperties> properties(queuesCount);
		vkGetPhysicalDeviceQueueFamilyProperties(pd, &queuesCount, &properties[0]);

		QueueFamilies qf{ -1 };
		for (size_t i = 0; i < properties.size(); ++i)
		{
			if (properties[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)
				qf.Graphics = i;

			VkBool32 presentSupport = false;
			vkGetPhysicalDeviceSurfaceSupportKHR(pd, i, app.Surface, &presentSupport);
			if (presentSupport)
				qf.Present = i;

			if (qf.IsComplete())
				break;
		}

		return qf;
	}

	bool IsDeviceSuitable(const VulkanApp& app, const VkPhysicalDevice& pd)
	{
		VkPhysicalDeviceProperties deviceProp;
		vkGetPhysicalDeviceProperties(pd, &deviceProp);

		auto details = QuerySwapChainDetails(app, pd);
		bool swapChainValid = !(details.PresentModes.empty() && details.Formats.empty());

		return FindQueueFamilies(app, pd).IsComplete()
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

		QueueFamilies queueFamilies = FindQueueFamilies(app, app.PhysicalDevice);

		std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
		std::set<int32_t> queueFamiliesSet = { queueFamilies.Graphics, queueFamilies.Present };

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

		vkGetDeviceQueue(app.Device, queueFamilies.Graphics, 0, &app.GraphicsQueue);
		vkGetDeviceQueue(app.Device, queueFamilies.Present, 0, &app.PresentQueue);
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

		auto families = FindQueueFamilies(app, app.PhysicalDevice);
		uint32_t queueFamilyIndices[] = { families.Graphics, families.Present };

		if (families.Graphics != families.Present)
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

	bool CreateShaderModule(const VulkanApp& app, const std::vector<char>& bytecode, VkShaderModule& outSM)
	{
		VkShaderModuleCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
		createInfo.codeSize = bytecode.size();
		createInfo.pCode = reinterpret_cast<const uint32_t*>(bytecode.data());

		if (vkCreateShaderModule(app.Device, &createInfo, nullptr, &outSM) != VK_SUCCESS)
			return false;

		return true;
	}

	std::vector<char> ReadFile(const std::string& filename) {
		std::ifstream file(filename, std::ios::ate | std::ios::binary);

		if (!file.is_open())
			throw std::runtime_error("failed to open file!");

		size_t fileSize = (size_t)file.tellg();
		std::vector<char> buffer(fileSize);

		file.seekg(0);
		file.read(buffer.data(), fileSize);

		file.close();

		return buffer;
	}

	bool CreatePipeline(VulkanApp& app)
	{
		auto vertexShader = ReadFile("shaders/vert.spv");
		auto fragmentShader = ReadFile("shaders/frag.spv");

		VkShaderModule vertexShaderModule;
		VkShaderModule fragmentShaderModule;

		if (!CreateShaderModule(app, vertexShader, vertexShaderModule))
			return false;
		if (!CreateShaderModule(app, fragmentShader, fragmentShaderModule))
			return false;

		VkPipelineShaderStageCreateInfo vertShaderPipelineCI{};
		vertShaderPipelineCI.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		vertShaderPipelineCI.stage = VK_SHADER_STAGE_VERTEX_BIT;
		vertShaderPipelineCI.module = vertexShaderModule;
		vertShaderPipelineCI.pName = "main";

		VkPipelineShaderStageCreateInfo fragShaderPipelineCI{};
		fragShaderPipelineCI.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		fragShaderPipelineCI.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
		fragShaderPipelineCI.module = fragmentShaderModule;
		fragShaderPipelineCI.pName = "main";

		VkPipelineShaderStageCreateInfo shaderStages[] = { vertShaderPipelineCI, fragShaderPipelineCI };

		VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
		vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
		vertexInputInfo.vertexBindingDescriptionCount = 0;
		vertexInputInfo.vertexAttributeDescriptionCount = 0;

		VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
		inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
		inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
		inputAssembly.primitiveRestartEnable = VK_FALSE;

		VkViewport viewport;
		viewport.x = 0.0f;
		viewport.y = 0.0f;
		viewport.width = (float)app.SwapChainExtent.width;
		viewport.height = (float)app.SwapChainExtent.height;
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;

		VkRect2D scissor{};
		scissor.offset = { 0, 0 };
		scissor.extent = app.SwapChainExtent;

		VkPipelineViewportStateCreateInfo viewportState{};
		viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
		viewportState.viewportCount = 1;
		viewportState.pViewports = &viewport;
		viewportState.scissorCount = 1;
		viewportState.pScissors = &scissor;

		VkPipelineRasterizationStateCreateInfo rasterizer{};
		rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
		rasterizer.depthClampEnable = VK_FALSE;
		rasterizer.rasterizerDiscardEnable = VK_FALSE;
		rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
		rasterizer.lineWidth = 1.0f;
		rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
		rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;
		rasterizer.depthBiasEnable = VK_FALSE;

		VkPipelineMultisampleStateCreateInfo multisample{};
		multisample.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
		multisample.sampleShadingEnable = VK_FALSE;
		multisample.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

		VkPipelineColorBlendAttachmentState colorBlendAttachment{};
		colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT
			| VK_COLOR_COMPONENT_G_BIT
			| VK_COLOR_COMPONENT_B_BIT
			| VK_COLOR_COMPONENT_A_BIT;
		colorBlendAttachment.blendEnable = VK_FALSE;

		VkPipelineColorBlendStateCreateInfo colorBlending{};
		colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
		colorBlending.logicOpEnable = VK_FALSE;
		colorBlending.logicOp = VK_LOGIC_OP_COPY;
		colorBlending.attachmentCount = 1;
		colorBlending.pAttachments = &colorBlendAttachment;
		colorBlending.blendConstants[0] = 0.0f;
		colorBlending.blendConstants[1] = 0.0f;
		colorBlending.blendConstants[2] = 0.0f;
		colorBlending.blendConstants[3] = 0.0f;

		VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
		pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipelineLayoutInfo.setLayoutCount = 0;
		pipelineLayoutInfo.pushConstantRangeCount = 0;

		if (vkCreatePipelineLayout(app.Device, &pipelineLayoutInfo, nullptr, &app.PipelineLayout) != VK_SUCCESS)
			return false;

		VkGraphicsPipelineCreateInfo pipelineInfo{};
		pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
		pipelineInfo.stageCount = 2;
		pipelineInfo.pStages = shaderStages;
		pipelineInfo.pVertexInputState = &vertexInputInfo;
		pipelineInfo.pInputAssemblyState = &inputAssembly;
		pipelineInfo.pViewportState = &viewportState;
		pipelineInfo.pRasterizationState = &rasterizer;
		pipelineInfo.pMultisampleState = &multisample;
		pipelineInfo.pColorBlendState = &colorBlending;
		pipelineInfo.pDynamicState = nullptr;
		pipelineInfo.layout = app.PipelineLayout;
		pipelineInfo.renderPass = app.RenderPass;
		pipelineInfo.subpass = 0;
		pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;

		if (vkCreateGraphicsPipelines(app.Device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &app.GraphicsPipeline) != VK_SUCCESS)
			return false;

		vkDestroyShaderModule(app.Device, vertexShaderModule, nullptr);
		vkDestroyShaderModule(app.Device, fragmentShaderModule, nullptr);


		app.Framebuffers.resize(app.SwapChainImageViews.size());

		for (size_t i = 0; i < app.SwapChainImageViews.size(); ++i)
		{
			VkFramebufferCreateInfo fboInfo{};
			fboInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
			fboInfo.renderPass = app.RenderPass;
			fboInfo.attachmentCount = 1;
			fboInfo.pAttachments = &app.SwapChainImageViews[i];
			fboInfo.width = app.SwapChainExtent.width;
			fboInfo.height = app.SwapChainExtent.height;
			fboInfo.layers = 1;

			if (vkCreateFramebuffer(app.Device, &fboInfo, nullptr, &app.Framebuffers[i]) != VK_SUCCESS)
				return false;
		}

		return true;
	}

	bool CreateRenderPass(VulkanApp& app)
	{
		VkAttachmentDescription colorAttachment{};
		colorAttachment.format = app.SwapChainFormat;
		colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
		colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

		VkAttachmentReference colorAttachmentRef{};
		colorAttachmentRef.attachment = 0;
		colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		VkSubpassDescription subpass{};
		subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
		subpass.colorAttachmentCount = 1;
		subpass.pColorAttachments = &colorAttachmentRef;

		VkRenderPassCreateInfo renderPassInfo{};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
		renderPassInfo.attachmentCount = 1;
		renderPassInfo.pAttachments = &colorAttachment;
		renderPassInfo.subpassCount = 1;
		renderPassInfo.pSubpasses = &subpass;

		VkSubpassDependency dependency{};
		dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
		dependency.dstSubpass = 0;
		dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		dependency.srcAccessMask = 0;
		dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

		renderPassInfo.dependencyCount = 1;
		renderPassInfo.pDependencies = &dependency;

		if (vkCreateRenderPass(app.Device, &renderPassInfo, nullptr, &app.RenderPass) != VK_SUCCESS)
			return false;

		return true;
	}

	bool SetupVulkanApp(const uint16_t width, const uint16_t height, VulkanApp& app)
	{
		if (!glfwInit())
			return false;

		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

		app.GlfwWindow = glfwCreateWindow(width, height, "Test", nullptr, nullptr);

		if (!app.GlfwWindow)
			return false;


		if (!CreateInstance(app))
			return false;

		if (glfwCreateWindowSurface(app.Instance, app.GlfwWindow, nullptr, &app.Surface) != VK_SUCCESS)
			return false;

		if (!SetupDevice(app))
			return false;

		if (!CreateSwapChain(app))
			return false;

		if (!CreateRenderPass(app))
			return false;

		if (!CreatePipeline(app))
			return false;


		QueueFamilies queueFamilies = FindQueueFamilies(app, app.PhysicalDevice);

		VkCommandPoolCreateInfo poolInfo{};
		poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		poolInfo.queueFamilyIndex = queueFamilies.Graphics;
		poolInfo.flags = 0;

		if (vkCreateCommandPool(app.Device, &poolInfo, nullptr, &app.CommandPool) != VK_SUCCESS)
			return false;

		app.CommandBuffers.resize(app.Framebuffers.size());

		VkCommandBufferAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocInfo.commandPool = app.CommandPool;
		allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		allocInfo.commandBufferCount = app.CommandBuffers.size();

		if (vkAllocateCommandBuffers(app.Device, &allocInfo, &app.CommandBuffers[0]) != VK_SUCCESS)
			return false;

		for (size_t i = 0; i < app.CommandBuffers.size(); ++i)
		{
			VkCommandBufferBeginInfo beginInfo{};
			beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
			beginInfo.flags = 0;
			beginInfo.pInheritanceInfo = nullptr;

			if (vkBeginCommandBuffer(app.CommandBuffers[i], &beginInfo) != VK_SUCCESS)
				return false;

			VkRenderPassBeginInfo renderPassInfo{};
			renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
			renderPassInfo.renderPass = app.RenderPass;
			renderPassInfo.framebuffer = app.Framebuffers[i];
			renderPassInfo.renderArea.offset = { 0, 0 };
			renderPassInfo.renderArea.extent = app.SwapChainExtent;

			VkClearValue clearColor = { 0.0f, 0.0f, 0.0f, 1.0f };
			renderPassInfo.clearValueCount = 1;
			renderPassInfo.pClearValues = &clearColor;

			vkCmdBeginRenderPass(app.CommandBuffers[i], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

			vkCmdBindPipeline(app.CommandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, app.GraphicsPipeline);

			vkCmdDraw(app.CommandBuffers[i], 3, 1, 0, 0);

			vkCmdEndRenderPass(app.CommandBuffers[i]);

			if (vkEndCommandBuffer(app.CommandBuffers[i]) != VK_SUCCESS)
				return false;
		}

		VkSemaphoreCreateInfo semaphoreInfo{};
		semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

		if (vkCreateSemaphore(app.Device, &semaphoreInfo, nullptr, &app.ImageAvailableSemaphore) != VK_SUCCESS
			|| vkCreateSemaphore(app.Device, &semaphoreInfo, nullptr, &app.RenderFinishedSemaphore) != VK_SUCCESS)
		{
			return false;
		}

		return true;
	}

	void CleanVulkanApp(VulkanApp& app)
	{
		vkDestroySemaphore(app.Device, app.ImageAvailableSemaphore, nullptr);
		vkDestroySemaphore(app.Device, app.RenderFinishedSemaphore, nullptr);

		vkDestroyCommandPool(app.Device, app.CommandPool, nullptr);

		for (const auto& fbo : app.Framebuffers)
			vkDestroyFramebuffer(app.Device, fbo, nullptr);

		vkDestroyPipeline(app.Device, app.GraphicsPipeline, nullptr);
		vkDestroyPipelineLayout(app.Device, app.PipelineLayout, nullptr);
		vkDestroyRenderPass(app.Device, app.RenderPass, nullptr);

		for (const auto& v : app.SwapChainImageViews)
			vkDestroyImageView(app.Device, v, nullptr);

		vkDestroySwapchainKHR(app.Device, app.SwapChain, nullptr);

		vkDestroyDevice(app.Device, nullptr);

		vkDestroySurfaceKHR(app.Instance, app.Surface, nullptr);

		DestroyDebugUtilsMessengerEXT(app.Instance, app.DebugMessenger, nullptr);

		vkDestroyInstance(app.Instance, nullptr);
	}

	bool DrawFrame(VulkanApp& app)
	{
		uint32_t imageId = 0;
		vkAcquireNextImageKHR(app.Device, app.SwapChain, UINT64_MAX, app.ImageAvailableSemaphore, VK_NULL_HANDLE, &imageId);

		VkSubmitInfo submitInfo{};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

		VkSemaphore waitSemaphores[] = { app.ImageAvailableSemaphore };
		VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };

		submitInfo.waitSemaphoreCount = 1;
		submitInfo.pWaitSemaphores = waitSemaphores;
		submitInfo.pWaitDstStageMask = waitStages;
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &app.CommandBuffers[imageId];

		VkSemaphore signalSemaphores[] = { app.RenderFinishedSemaphore };
		submitInfo.signalSemaphoreCount = 1;
		submitInfo.pSignalSemaphores = signalSemaphores;

		if (vkQueueSubmit(app.GraphicsQueue, 1, &submitInfo, VK_NULL_HANDLE) != VK_SUCCESS)
			return false;

		VkPresentInfoKHR presentInfo{};
		presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
		presentInfo.waitSemaphoreCount = 1;
		presentInfo.pWaitSemaphores = signalSemaphores;

		VkSwapchainKHR swapChains[] = { app.SwapChain };
		presentInfo.swapchainCount = 1;
		presentInfo.pSwapchains = swapChains;
		presentInfo.pImageIndices = &imageId;

		vkQueuePresentKHR(app.PresentQueue, &presentInfo);
		vkQueueWaitIdle(app.PresentQueue);
	}

	void RunVulkanApp(VulkanApp& app, const std::function<void()>& callback)
	{
		while (!glfwWindowShouldClose(app.GlfwWindow))
		{
			glfwPollEvents();

			callback();

			DrawFrame(app);
		}
	}
}