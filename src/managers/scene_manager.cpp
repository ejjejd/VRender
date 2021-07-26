#include "scene_manager.h"

namespace manager
{
	bool SceneManager::CreatePipeline(VkPipeline& pipeline, VkPipelineLayout& pipelineLayout, graphics::Shader& shader)
	{
		VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
		inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
		inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
		inputAssembly.primitiveRestartEnable = VK_FALSE;

		VkViewport viewport;
		viewport.x = 0.0f;
		viewport.y = 0.0f;
		viewport.width = (float)VulkanApp->SwapChainExtent.width;
		viewport.height = (float)VulkanApp->SwapChainExtent.height;
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;

		VkRect2D scissor{};
		scissor.offset = { 0, 0 };
		scissor.extent = VulkanApp->SwapChainExtent;

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

		if (vkCreatePipelineLayout(VulkanApp->Device, &pipelineLayoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS)
			return false;

		VkGraphicsPipelineCreateInfo pipelineInfo{};
		pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
		pipelineInfo.stageCount = shader.GetStages().size();
		pipelineInfo.pStages = shader.GetStages().data();
		pipelineInfo.pVertexInputState = &shader.GetInputState();
		pipelineInfo.pInputAssemblyState = &inputAssembly;
		pipelineInfo.pViewportState = &viewportState;
		pipelineInfo.pRasterizationState = &rasterizer;
		pipelineInfo.pMultisampleState = &multisample;
		pipelineInfo.pColorBlendState = &colorBlending;
		pipelineInfo.pDynamicState = nullptr;
		pipelineInfo.layout = pipelineLayout;
		pipelineInfo.renderPass = RM->GetRenderPass();
		pipelineInfo.subpass = 0;
		pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;

		if (vkCreateGraphicsPipelines(VulkanApp->Device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &pipeline) != VK_SUCCESS)
			return false;

		return true;
	}

	bool SceneManager::CreateCommandBuffers(const graphics::Buffer& buffer, const VkPipeline& pipeline, std::vector<VkCommandBuffer>& commandBuffers)
	{
		vk::VulkanQueueFamilies queueFamilies = VulkanApp->QueueFamilies;

		VkCommandPoolCreateInfo poolInfo{};
		poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		poolInfo.queueFamilyIndex = queueFamilies.Graphics;
		poolInfo.flags = 0;

		if (vkCreateCommandPool(VulkanApp->Device, &poolInfo, nullptr, &CommandPool) != VK_SUCCESS)
			return false;

		commandBuffers.resize(RM->GetFBOs().size());

		VkCommandBufferAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocInfo.commandPool = CommandPool;
		allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		allocInfo.commandBufferCount = commandBuffers.size();

		if (vkAllocateCommandBuffers(VulkanApp->Device, &allocInfo, &commandBuffers[0]) != VK_SUCCESS)
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
			renderPassInfo.renderPass = RM->GetRenderPass();
			renderPassInfo.framebuffer = RM->GetFBOs()[i];
			renderPassInfo.renderArea.offset = { 0, 0 };
			renderPassInfo.renderArea.extent = VulkanApp->SwapChainExtent;

			VkClearValue clearColor = { 0.0f, 0.0f, 0.0f, 1.0f };
			renderPassInfo.clearValueCount = 1;
			renderPassInfo.pClearValues = &clearColor;

			vkCmdBeginRenderPass(commandBuffers[i], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

			vkCmdBindPipeline(commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);

			VkBuffer vertexBuffers[] = { buffer.GetHandler() };
			VkDeviceSize offsets[] = { 0 };
			vkCmdBindVertexBuffers(commandBuffers[i], 0, 1, vertexBuffers, offsets);

			vkCmdDraw(commandBuffers[i], buffer.GetElementsCount(), 1, 0, 0);

			vkCmdEndRenderPass(commandBuffers[i]);

			if (vkEndCommandBuffer(commandBuffers[i]) != VK_SUCCESS)
				return false;
		}

		return true;
	}

	void SceneManager::Cleanup()
	{
		for (auto vbo : MeshBuffers)
			vbo.Cleanup();

		for (auto r : Renderables)
			render::CleanupRenderable(*VulkanApp, CommandPool, r);

		vkDestroyCommandPool(VulkanApp->Device, CommandPool, nullptr);
	}

	void SceneManager::RegisterMesh(render::Mesh& mesh)
	{
		RegisteredMeshes.push_back(mesh);

		render::Renderable renderable;

		graphics::Shader shader;
		shader.Setup(*VulkanApp);

		shader.AddStage(mesh.VertexShader, VK_SHADER_STAGE_VERTEX_BIT);
		shader.AddStage(mesh.FragmentShader, VK_SHADER_STAGE_FRAGMENT_BIT);

		graphics::Buffer positionBuffer;
		positionBuffer.Setup(*VulkanApp , &mesh.Positions[0], sizeof(mesh.Positions[0]), mesh.Positions.size());

		shader.AddInputBuffer(VK_FORMAT_R32G32B32_SFLOAT, 0, 0, 0, positionBuffer.GetStride());

		if (!CreatePipeline(renderable.GraphicsPipeline, renderable.GraphicsPipelineLayout, shader))
			return;

		if (!CreateCommandBuffers(positionBuffer, renderable.GraphicsPipeline, renderable.CommandBuffers))
			return;

		MeshBuffers.push_back(positionBuffer);

		Renderables.push_back(renderable);

		shader.Cleanup();
	}
}