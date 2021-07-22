#define GLFW_INCLUDE_VULKAN
#include "GLFW/glfw3.h"

#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"

#include <iostream>
#include <vector>
#include <set>
#include <algorithm>
#include <fstream>

GLFWwindow* window;

constexpr uint16_t width = 1280;
constexpr uint16_t height = 720;

constexpr bool enableValidationLayers = true;

const std::vector<const char*> validationLayers =
{
	"VK_LAYER_KHRONOS_validation"
};

const std::vector<const char*> deviceExtensions =
{
	VK_KHR_SWAPCHAIN_EXTENSION_NAME
};

VkInstance vulkanInst;

VkDebugUtilsMessengerEXT debugMessenger;

VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;

VkDevice device;

VkQueue graphicsQueue;
VkQueue presentQueue;

VkSurfaceKHR surface;

VkSwapchainKHR swapChain;

std::vector<VkImage> swapChainImages;
std::vector<VkImageView> swapChainImageViews;
VkFormat swapChainFormat;
VkExtent2D swapChainExtent;

VkRenderPass renderPass;
VkPipelineLayout pipelineLayout;

VkPipeline graphicsPipeline;

std::vector<VkFramebuffer> framebuffers;

VkCommandPool commandPool;

std::vector<VkCommandBuffer> commandBuffers;

VkSemaphore imageAvailableSemaphore;
VkSemaphore renderFinishedSemaphore;

VKAPI_ATTR VkBool32 VKAPI_CALL DebugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
													VkDebugUtilsMessageTypeFlagsEXT messageType,
													const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
													void* pUserData) 
{
	if (messageSeverity <= VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT)
		return VK_FALSE;

	std::cerr << "Validation layer: " << pCallbackData->pMessage << std::endl;

	return VK_FALSE;
}

bool CheckLayers()
{
	uint32_t layersCount;
	vkEnumerateInstanceLayerProperties(&layersCount, nullptr);

	std::vector<VkLayerProperties> availableLayers(layersCount);
	vkEnumerateInstanceLayerProperties(&layersCount, &availableLayers[0]);

	for (auto l : validationLayers)
	{
		bool found = false;
		for (auto aL : availableLayers)
		{
			if (strcmp(l, aL.layerName) == 0)
				found = true;
		}

		if (!found)
			return false;
	}
}

std::vector<const char*> GetRequiredExtensions()
{
	uint32_t extensionsCount = 0;
	const char** glfwExtensions;

	glfwExtensions = glfwGetRequiredInstanceExtensions(&extensionsCount);

	std::vector<const char*> extensions(glfwExtensions, glfwExtensions + extensionsCount);

	if(enableValidationLayers)
		extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);

	return extensions;
}

bool CheckDeviceExtensions(VkPhysicalDevice pd)
{
	uint32_t extensionsCount = 0;
	vkEnumerateDeviceExtensionProperties(pd, nullptr, &extensionsCount, nullptr);

	std::vector<VkExtensionProperties> availableExtensions(extensionsCount);
	vkEnumerateDeviceExtensionProperties(pd, nullptr, &extensionsCount, &availableExtensions[0]);

	std::set<std::string> requiredExtensions(deviceExtensions.begin(), deviceExtensions.end());

	for (const auto& e : availableExtensions)
		requiredExtensions.erase(e.extensionName);

	return requiredExtensions.empty();
}

struct QueueFamilies
{
	int32_t Graphics;
	int32_t Present;

	bool IsComplete() const
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

SwapChainDetails QuerySwapChainDetails(VkPhysicalDevice pd)
{
	SwapChainDetails details;

	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(pd, surface, &details.Capabilities);


	uint32_t formatsCount = 0;
	vkGetPhysicalDeviceSurfaceFormatsKHR(pd, surface, &formatsCount, nullptr);

	details.Formats.resize(formatsCount);
	vkGetPhysicalDeviceSurfaceFormatsKHR(pd, surface, &formatsCount, &details.Formats[0]);


	uint32_t presentModesCount = 0;
	vkGetPhysicalDeviceSurfacePresentModesKHR(pd, surface, &presentModesCount, nullptr);

	details.PresentModes.resize(presentModesCount);
	vkGetPhysicalDeviceSurfacePresentModesKHR(pd, surface, &presentModesCount, &details.PresentModes[0]);

	return details;
}

QueueFamilies FindQueueFamilies(VkPhysicalDevice pd)
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
		vkGetPhysicalDeviceSurfaceSupportKHR(pd, i, surface, &presentSupport);
		if (presentSupport)
			qf.Present = i;

		if (qf.IsComplete())
			break;
	}

	return qf;
}

bool IsDeviceSuitable(const VkPhysicalDevice& pd)
{
	VkPhysicalDeviceProperties deviceProp;
	vkGetPhysicalDeviceProperties(pd, &deviceProp);

	auto details = QuerySwapChainDetails(pd);
	bool swapChainValid = !(details.PresentModes.empty() && details.Formats.empty());

	return FindQueueFamilies(pd).IsComplete() 
		   && CheckDeviceExtensions(pd)
		   && swapChainValid;
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

VkExtent2D ChooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities)
{
	if (capabilities.currentExtent.width != UINT32_MAX)
		return capabilities.currentExtent;

	int width, height;
	glfwGetFramebufferSize(window, &width, &height);

	VkExtent2D extent{ 0 };
	extent.width = std::clamp((uint32_t)width, capabilities.minImageExtent.width, capabilities.maxImageExtent.height);
	extent.height = std::clamp((uint32_t)height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

	return extent;
}

bool CreateSwapChain()
{
	auto details = QuerySwapChainDetails(physicalDevice);

	auto surfaceFormat = ChooseSwapSurfaceFormat(details.Formats);

	auto presentMode = ChooseSwapPresentMode(details.PresentModes);

	auto extent = ChooseSwapExtent(details.Capabilities);

	uint32_t imageCount = details.Capabilities.maxImageCount;

	VkSwapchainCreateInfoKHR createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	createInfo.surface = surface;
	createInfo.minImageCount = imageCount;
	createInfo.imageFormat = surfaceFormat.format;
	createInfo.imageColorSpace = surfaceFormat.colorSpace;
	createInfo.imageExtent = extent;
	createInfo.imageArrayLayers = 1;
	createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

	auto families = FindQueueFamilies(physicalDevice);
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

	if (vkCreateSwapchainKHR(device, &createInfo, nullptr, &swapChain) != VK_SUCCESS)
		return false;

	swapChainFormat = surfaceFormat.format;
	swapChainExtent = extent;

	vkGetSwapchainImagesKHR(device, swapChain, &imageCount, nullptr);
	swapChainImages.resize(imageCount);
	vkGetSwapchainImagesKHR(device, swapChain, &imageCount, &swapChainImages[0]);

	swapChainImageViews.resize(imageCount);

	for (size_t i = 0; i < imageCount; ++i)
	{
		VkImageViewCreateInfo createInfo{};

		createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		createInfo.image = swapChainImages[i];
		createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		createInfo.format = swapChainFormat;

		createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;

		createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		createInfo.subresourceRange.baseMipLevel = 0;
		createInfo.subresourceRange.levelCount = 1;
		createInfo.subresourceRange.baseArrayLayer = 0;
		createInfo.subresourceRange.layerCount = 1;

		if (vkCreateImageView(device, &createInfo, nullptr, &swapChainImageViews[i]) != VK_SUCCESS)
			return false;
	}

	return true;
}

bool CreateShaderModule(const std::vector<char>& bytecode, VkShaderModule& outSM)
{
	VkShaderModuleCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	createInfo.codeSize = bytecode.size();
	createInfo.pCode = reinterpret_cast<const uint32_t*>(bytecode.data());

	if (vkCreateShaderModule(device, &createInfo, nullptr, &outSM) != VK_SUCCESS)
		return false;

	return true;
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

bool Startup()
{
	if (!glfwInit())
		return false;

	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

	window = glfwCreateWindow(width, height, "Test", nullptr, nullptr);

	if (!window)
		return false;


	VkApplicationInfo appInfo{};
	appInfo.pApplicationName = "Test";
	appInfo.pEngineName = "Test";
	appInfo.apiVersion = VK_API_VERSION_1_0;
	appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
	appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
	appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	appInfo.pNext = nullptr;

	VkInstanceCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	createInfo.pApplicationInfo = &appInfo;

	auto extensions = GetRequiredExtensions();
	createInfo.ppEnabledExtensionNames = extensions.data();
	createInfo.enabledExtensionCount = extensions.size();

	createInfo.ppEnabledLayerNames = validationLayers.data();
	createInfo.enabledLayerCount = validationLayers.size();

	if (vkCreateInstance(&createInfo, nullptr, &vulkanInst) != VK_SUCCESS)
		return false;

	VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo{};
	debugCreateInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
	debugCreateInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT
									  | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT
									  | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
	debugCreateInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT
								  | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT
								  | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
	debugCreateInfo.pfnUserCallback = DebugCallback;
	debugCreateInfo.pUserData = nullptr;

	
	if (enableValidationLayers)
	{
		if (CreateDebugUtilsMessengerEXT(vulkanInst, &debugCreateInfo, nullptr, &debugMessenger) != VK_SUCCESS)
			return false;
	}


	if (glfwCreateWindowSurface(vulkanInst, window, nullptr, &surface) != VK_SUCCESS)
		return false;

	uint32_t physicalDevicesCount = 0;
	vkEnumeratePhysicalDevices(vulkanInst, &physicalDevicesCount, nullptr);

	if (physicalDevicesCount == 0)
		return false;

	std::vector<VkPhysicalDevice> physicalDevices(physicalDevicesCount);
	vkEnumeratePhysicalDevices(vulkanInst, &physicalDevicesCount, &physicalDevices[0]);

	for (const auto& pd : physicalDevices)
	{
		if (IsDeviceSuitable(pd))
		{
			physicalDevice = pd;
			break;
		}
	}
	
	if (physicalDevice == VK_NULL_HANDLE)
		return false;


	if (!CheckLayers())
		return false;


	QueueFamilies queueFamilies = FindQueueFamilies(physicalDevice);

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
	deviceCreateInfo.ppEnabledExtensionNames = deviceExtensions.data();
	deviceCreateInfo.enabledExtensionCount = deviceExtensions.size();

	if (vkCreateDevice(physicalDevice, &deviceCreateInfo, nullptr, &device) != VK_SUCCESS)
		return false;
	
	vkGetDeviceQueue(device, queueFamilies.Graphics, 0, &graphicsQueue);
	vkGetDeviceQueue(device, queueFamilies.Present, 0, &presentQueue);

	if (!CreateSwapChain())
		return false;

	VkAttachmentDescription colorAttachment{};
	colorAttachment.format = swapChainFormat;
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

	if (vkCreateRenderPass(device, &renderPassInfo, nullptr, &renderPass) != VK_SUCCESS)
		return false;

	auto vertexShader = ReadFile("shaders/vert.spv");
	auto fragmentShader = ReadFile("shaders/frag.spv");

	VkShaderModule vertexShaderModule;
	VkShaderModule fragmentShaderModule;

	if (!CreateShaderModule(vertexShader, vertexShaderModule))
		return false;
	if(!CreateShaderModule(fragmentShader, fragmentShaderModule))
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
	viewport.width = (float)swapChainExtent.width;
	viewport.height = (float)swapChainExtent.height;
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;

	VkRect2D scissor{};
	scissor.offset = { 0, 0 };
	scissor.extent = swapChainExtent;

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

	if (vkCreatePipelineLayout(device, &pipelineLayoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS)
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
	pipelineInfo.layout = pipelineLayout;
	pipelineInfo.renderPass = renderPass;
	pipelineInfo.subpass = 0;
	pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;

	if (vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &graphicsPipeline) != VK_SUCCESS)
		return false;

	vkDestroyShaderModule(device, vertexShaderModule, nullptr);
	vkDestroyShaderModule(device, fragmentShaderModule, nullptr);


	framebuffers.resize(swapChainImageViews.size());

	for (size_t i = 0; i < swapChainImageViews.size(); ++i)
	{
		VkFramebufferCreateInfo fboInfo{};
		fboInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		fboInfo.renderPass = renderPass;
		fboInfo.attachmentCount = 1;
		fboInfo.pAttachments = &swapChainImageViews[i];
		fboInfo.width = swapChainExtent.width;
		fboInfo.height = swapChainExtent.height;
		fboInfo.layers = 1;

		if (vkCreateFramebuffer(device, &fboInfo, nullptr, &framebuffers[i]) != VK_SUCCESS)
			return false;
	}


	VkCommandPoolCreateInfo poolInfo{};
	poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	poolInfo.queueFamilyIndex = queueFamilies.Graphics;
	poolInfo.flags = 0;

	if (vkCreateCommandPool(device, &poolInfo, nullptr, &commandPool) != VK_SUCCESS)
		return false;

	commandBuffers.resize(framebuffers.size());

	VkCommandBufferAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.commandPool = commandPool;
	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocInfo.commandBufferCount = commandBuffers.size();

	if (vkAllocateCommandBuffers(device, &allocInfo, &commandBuffers[0]) != VK_SUCCESS)
		return false;

	for (size_t i = 0; i < commandBuffers.size(); ++i)
	{
		VkCommandBufferBeginInfo beginInfo{};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		beginInfo.flags = 0;
		beginInfo.pInheritanceInfo = nullptr;

		if (vkBeginCommandBuffer(commandBuffers[i], &beginInfo) != VK_SUCCESS)
			return false;

		VkRenderPassBeginInfo renderPassInfo{};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderPassInfo.renderPass = renderPass;
		renderPassInfo.framebuffer = framebuffers[i];
		renderPassInfo.renderArea.offset = { 0, 0 };
		renderPassInfo.renderArea.extent = swapChainExtent;

		VkClearValue clearColor = { 0.0f, 0.0f, 0.0f, 1.0f };
		renderPassInfo.clearValueCount = 1;
		renderPassInfo.pClearValues = &clearColor;

		vkCmdBeginRenderPass(commandBuffers[i], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

		vkCmdBindPipeline(commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipeline);

		vkCmdDraw(commandBuffers[i], 3, 1, 0, 0);

		vkCmdEndRenderPass(commandBuffers[i]);

		if (vkEndCommandBuffer(commandBuffers[i]) != VK_SUCCESS)
			return false;
	}


	VkSemaphoreCreateInfo semaphoreInfo{};
	semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

	if (vkCreateSemaphore(device, &semaphoreInfo, nullptr, &imageAvailableSemaphore) != VK_SUCCESS
		|| vkCreateSemaphore(device, &semaphoreInfo, nullptr, &renderFinishedSemaphore) != VK_SUCCESS)
	{
		return false;
	}

	return true;
}

void Cleanup()
{
	vkDestroySemaphore(device, imageAvailableSemaphore, nullptr);
	vkDestroySemaphore(device, renderFinishedSemaphore, nullptr);

	vkDestroyCommandPool(device, commandPool, nullptr);

	for (auto fbo : framebuffers)
		vkDestroyFramebuffer(device, fbo, nullptr);

	vkDestroyPipeline(device, graphicsPipeline, nullptr);
	vkDestroyPipelineLayout(device, pipelineLayout, nullptr);
	vkDestroyRenderPass(device, renderPass, nullptr);

	for (auto imageV : swapChainImageViews)
		vkDestroyImageView(device, imageV, nullptr);

	vkDestroySwapchainKHR(device, swapChain, nullptr);

	vkDestroyDevice(device, nullptr);

	if(enableValidationLayers)
		DestroyDebugUtilsMessengerEXT(vulkanInst, debugMessenger, nullptr);

	vkDestroySurfaceKHR(vulkanInst, surface, nullptr);

	vkDestroyInstance(vulkanInst, nullptr);

	glfwTerminate();
}

bool DrawFrame()
{
	uint32_t imageId = 0;
	vkAcquireNextImageKHR(device, swapChain, UINT64_MAX, imageAvailableSemaphore, VK_NULL_HANDLE, &imageId);

	VkSubmitInfo submitInfo{};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

	VkSemaphore waitSemaphores[] = { imageAvailableSemaphore };
	VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };

	submitInfo.waitSemaphoreCount = 1;
	submitInfo.pWaitSemaphores = waitSemaphores;
	submitInfo.pWaitDstStageMask = waitStages;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &commandBuffers[imageId];

	VkSemaphore signalSemaphores[] = { renderFinishedSemaphore };
	submitInfo.signalSemaphoreCount = 1;
	submitInfo.pSignalSemaphores = signalSemaphores;

	if (vkQueueSubmit(graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE) != VK_SUCCESS)
		return false;

	VkPresentInfoKHR presentInfo{};
	presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	presentInfo.waitSemaphoreCount = 1;
	presentInfo.pWaitSemaphores = signalSemaphores;

	VkSwapchainKHR swapChains[] = { swapChain };
	presentInfo.swapchainCount = 1;
	presentInfo.pSwapchains = swapChains;
	presentInfo.pImageIndices = &imageId;

	vkQueuePresentKHR(presentQueue, &presentInfo);
	vkQueueWaitIdle(presentQueue);
}
void MainLoop()
{
	while (!glfwWindowShouldClose(window))
	{
		glfwPollEvents();

		DrawFrame();
	}

	vkDeviceWaitIdle(device);
}


int main()
{
	if (!Startup())
		return -1;

	MainLoop();

	Cleanup();

	return 0;
}