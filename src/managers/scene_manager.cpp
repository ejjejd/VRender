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

	void SceneManager::Cleanup()
	{
		for (auto vbo : MeshBuffers)
			vbo.Cleanup();

		for (auto r : Renderables)
			render::CleanupRenderable(*VulkanApp, r);
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
		positionBuffer.Setup(*VulkanApp , VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, sizeof(mesh.MeshInfo.Positions[0]), mesh.MeshInfo.Positions.size());
		positionBuffer.Update(&mesh.MeshInfo.Positions[0], mesh.MeshInfo.Positions.size());

		renderable.PositionsVertexBuffer = positionBuffer;

		shader.AddInputBuffer(VK_FORMAT_R32G32B32_SFLOAT, 0, 0, 0, positionBuffer.GetStride());

		if (!CreatePipeline(renderable.GraphicsPipeline, renderable.GraphicsPipelineLayout, shader))
			return;

		MeshBuffers.push_back(positionBuffer);

		Renderables.push_back(renderable);

		shader.Cleanup();
	}
}