#include "render_manager.h"

#include <set>
#include <fstream>

namespace manager
{
	struct QueueFamilies
	{
		int32_t Graphics;
		int32_t Present;

		inline bool IsComplete() const
		{
			return Graphics != -1 && Present != -1;
		}
	};

	QueueFamilies FindQueueFamilies(const vk::VulkanApp& app, VkPhysicalDevice pd)
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

	bool CreateShaderModule(const vk::VulkanApp& app, const std::vector<char>& bytecode, VkShaderModule& outSM)
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
	
	int32_t FindMemoryType(const vk::VulkanApp& app, uint32_t typeFilter, VkMemoryPropertyFlags properties)
	{
		VkPhysicalDeviceMemoryProperties memProperties;
		vkGetPhysicalDeviceMemoryProperties(app.PhysicalDevice, &memProperties);
	
		for (uint32_t i = 0; i < memProperties.memoryTypeCount; ++i)
		{
			if ((memProperties.memoryTypes[i].propertyFlags & properties) == properties
				&& typeFilter & (1 << i))
			{
				return i;
			}
		}
	
		return -1;
	}

	bool RenderManager::CreatePipeline(vk::VulkanApp& app)
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


		glm::vec3 positions[] =
		{
			{ -0.5f, 0.5f, 0.0f },
			{ 0.0f, -0.5f, 0.0f },
			{ 0.5f, 0.5f, 0.0f }
		};

		VkVertexInputBindingDescription inputDescription{};
		inputDescription.binding = 0;
		inputDescription.stride = sizeof(glm::vec3);
		inputDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

		VkVertexInputAttributeDescription attributeDescription{};
		attributeDescription.binding = 0;
		attributeDescription.location = 0;
		attributeDescription.format = VK_FORMAT_R32G32B32_SFLOAT;
		attributeDescription.offset = 0;

		VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
		vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
		vertexInputInfo.vertexBindingDescriptionCount = 1;
		vertexInputInfo.pVertexBindingDescriptions = &inputDescription;
		vertexInputInfo.vertexAttributeDescriptionCount = 1;
		vertexInputInfo.pVertexAttributeDescriptions = &attributeDescription;

		VkBufferCreateInfo bufferCreateInfo{};
		bufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		bufferCreateInfo.size = sizeof(glm::vec3) * 3;
		bufferCreateInfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
		bufferCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

		if (vkCreateBuffer(app.Device, &bufferCreateInfo, nullptr, &VertexBuffer) != VK_SUCCESS)
			return false;

		VkMemoryRequirements memRequirements;
		vkGetBufferMemoryRequirements(app.Device, VertexBuffer, &memRequirements);

		VkMemoryAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		allocInfo.allocationSize = memRequirements.size;
		allocInfo.memoryTypeIndex = FindMemoryType(app, memRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT
			| VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

		if (vkAllocateMemory(app.Device, &allocInfo, nullptr, &VertexBufferMemory) != VK_SUCCESS)
			return false;

		vkBindBufferMemory(app.Device, VertexBuffer, VertexBufferMemory, 0);

		void* data;
		vkMapMemory(app.Device, VertexBufferMemory, 0, bufferCreateInfo.size, 0, &data);
		memcpy(data, positions, bufferCreateInfo.size);
		vkUnmapMemory(app.Device, VertexBufferMemory);

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

		if (vkCreatePipelineLayout(app.Device, &pipelineLayoutInfo, nullptr, &PipelineLayout) != VK_SUCCESS)
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
		pipelineInfo.layout = PipelineLayout;
		pipelineInfo.renderPass = RenderPass;
		pipelineInfo.subpass = 0;
		pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;

		if (vkCreateGraphicsPipelines(app.Device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &GraphicsPipeline) != VK_SUCCESS)
			return false;

		vkDestroyShaderModule(app.Device, vertexShaderModule, nullptr);
		vkDestroyShaderModule(app.Device, fragmentShaderModule, nullptr);

		return true;
	}

	bool RenderManager::CreateRenderPass(vk::VulkanApp& app)
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

		if (vkCreateRenderPass(app.Device, &renderPassInfo, nullptr, &RenderPass) != VK_SUCCESS)
			return false;

		Framebuffers.resize(app.SwapChainImageViews.size());

		for (size_t i = 0; i < app.SwapChainImageViews.size(); ++i)
		{
			VkFramebufferCreateInfo fboInfo{};
			fboInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
			fboInfo.renderPass = RenderPass;
			fboInfo.attachmentCount = 1;
			fboInfo.pAttachments = &app.SwapChainImageViews[i];
			fboInfo.width = app.SwapChainExtent.width;
			fboInfo.height = app.SwapChainExtent.height;
			fboInfo.layers = 1;

			if (vkCreateFramebuffer(app.Device, &fboInfo, nullptr, &Framebuffers[i]) != VK_SUCCESS)
				return false;
		}

		return true;
	}

	bool RenderManager::Setup(vk::VulkanApp& app)
	{
		if (!CreateRenderPass(app))
			return false;

		if (!CreatePipeline(app))
			return false;

		QueueFamilies queueFamilies = FindQueueFamilies(app, app.PhysicalDevice);

		VkCommandPoolCreateInfo poolInfo{};
		poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		poolInfo.queueFamilyIndex = queueFamilies.Graphics;
		poolInfo.flags = 0;

		if (vkCreateCommandPool(app.Device, &poolInfo, nullptr, &CommandPool) != VK_SUCCESS)
			return false;

		CommandBuffers.resize(Framebuffers.size());

		VkCommandBufferAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocInfo.commandPool = CommandPool;
		allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		allocInfo.commandBufferCount = CommandBuffers.size();

		if (vkAllocateCommandBuffers(app.Device, &allocInfo, &CommandBuffers[0]) != VK_SUCCESS)
			return false;

		for (size_t i = 0; i < CommandBuffers.size(); ++i)
		{
			VkCommandBufferBeginInfo beginInfo{};
			beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
			beginInfo.flags = 0;
			beginInfo.pInheritanceInfo = nullptr;

			if (vkBeginCommandBuffer(CommandBuffers[i], &beginInfo) != VK_SUCCESS)
				return false;

			VkRenderPassBeginInfo renderPassInfo{};
			renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
			renderPassInfo.renderPass = RenderPass;
			renderPassInfo.framebuffer = Framebuffers[i];
			renderPassInfo.renderArea.offset = { 0, 0 };
			renderPassInfo.renderArea.extent = app.SwapChainExtent;

			VkClearValue clearColor = { 0.0f, 0.0f, 0.0f, 1.0f };
			renderPassInfo.clearValueCount = 1;
			renderPassInfo.pClearValues = &clearColor;

			vkCmdBeginRenderPass(CommandBuffers[i], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

			vkCmdBindPipeline(CommandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, GraphicsPipeline);

			VkBuffer vertexBuffers[] = { VertexBuffer };
			VkDeviceSize offsets[] = { 0 };
			vkCmdBindVertexBuffers(CommandBuffers[i], 0, 1, vertexBuffers, offsets);

			vkCmdDraw(CommandBuffers[i], 3, 1, 0, 0);

			vkCmdEndRenderPass(CommandBuffers[i]);

			if (vkEndCommandBuffer(CommandBuffers[i]) != VK_SUCCESS)
				return false;
		}

		VkSemaphoreCreateInfo semaphoreInfo{};
		semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

		if (vkCreateSemaphore(app.Device, &semaphoreInfo, nullptr, &ImageAvailableSemaphore) != VK_SUCCESS
			|| vkCreateSemaphore(app.Device, &semaphoreInfo, nullptr, &RenderFinishedSemaphore) != VK_SUCCESS)
		{
			return false;
		}

		return true;
	}

	void RenderManager::Cleanup(vk::VulkanApp& app)
	{
		for (size_t i = 0; i < Framebuffers.size(); i++)
			vkDestroyFramebuffer(app.Device, Framebuffers[i], nullptr);

		vkFreeCommandBuffers(app.Device, CommandPool, static_cast<uint32_t>(CommandBuffers.size()), CommandBuffers.data());

		vkDestroyCommandPool(app.Device, CommandPool, nullptr);

		vkDestroyPipeline(app.Device, GraphicsPipeline, nullptr);
		vkDestroyPipelineLayout(app.Device, PipelineLayout, nullptr);
		vkDestroyRenderPass(app.Device, RenderPass, nullptr);

		vkDestroyBuffer(app.Device, VertexBuffer, nullptr);
		vkFreeMemory(app.Device, VertexBufferMemory, nullptr);

		vkDestroySemaphore(app.Device, ImageAvailableSemaphore, nullptr);
		vkDestroySemaphore(app.Device, RenderFinishedSemaphore, nullptr);
	}

	bool RenderManager::Update(vk::VulkanApp& app)
	{
		uint32_t imageId = 0;
		VkResult acqResult = vkAcquireNextImageKHR(app.Device, app.SwapChain, UINT64_MAX, ImageAvailableSemaphore, VK_NULL_HANDLE, &imageId);

		VkSubmitInfo submitInfo{};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

		VkSemaphore waitSemaphores[] = { ImageAvailableSemaphore };
		VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };

		submitInfo.waitSemaphoreCount = 1;
		submitInfo.pWaitSemaphores = waitSemaphores;
		submitInfo.pWaitDstStageMask = waitStages;
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &CommandBuffers[imageId];

		VkSemaphore signalSemaphores[] = { RenderFinishedSemaphore };
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

		return true;
	}
}