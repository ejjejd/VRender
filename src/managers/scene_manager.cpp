#include "scene_manager.h"

namespace manager
{
	struct PointLightUBO
	{
		glm::vec4 Position;
		glm::vec4 Color;
	};

	struct alignas(16) SpotlightUBO
	{
		glm::vec4 Position;
		glm::vec4 Direction;
		glm::vec4 Color;

		float OuterAngle;
		float InnerAngle;
	};

	struct LightDataUBO
	{
		PointLightUBO PointLights[MaxPointLights + 1];
		SpotlightUBO Spotlights[MaxSpotlights + 1];
		int PointLightsCount;
		int SpotlightsCount;
	};

	struct MeshUBO
	{
		glm::mat4 Transform;
	};

	bool SceneManager::CreatePipeline(const std::vector<VkDescriptorSetLayout>& layouts, 
									  VkPipeline& pipeline, VkPipelineLayout& pipelineLayout, vk::Shader& shader)
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

		VkPipelineDepthStencilStateCreateInfo depthState{};
		depthState.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
		depthState.depthTestEnable = VK_TRUE;
		depthState.depthWriteEnable = VK_TRUE;
		depthState.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;
		depthState.depthBoundsTestEnable = VK_FALSE;
		depthState.minDepthBounds = 0.0f;
		depthState.maxDepthBounds = 1.0f;
		depthState.stencilTestEnable = VK_FALSE;

		VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
		pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipelineLayoutInfo.setLayoutCount = layouts.size();
		pipelineLayoutInfo.pSetLayouts = layouts.data();

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
		pipelineInfo.pDepthStencilState = &depthState;
		pipelineInfo.pDynamicState = nullptr;
		pipelineInfo.layout = pipelineLayout;
		pipelineInfo.renderPass = RM->GetRenderPass();
		pipelineInfo.subpass = 0;
		pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;

		if (vkCreateGraphicsPipelines(VulkanApp->Device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &pipeline) != VK_SUCCESS)
			return false;

		return true;
	}

	void SceneManager::Setup(vk::VulkanApp& app, RenderManager& rm)
	{
		VulkanApp = &app;
		RM = &rm;

		VkDescriptorPoolSize poolSize{};
		poolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		poolSize.descriptorCount = RM->GetFBOs().size() * 255;

		VkDescriptorPoolCreateInfo poolCreateInfo{};
		poolCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		poolCreateInfo.poolSizeCount = 1;
		poolCreateInfo.pPoolSizes = &poolSize;
		poolCreateInfo.maxSets = 255;

		if (vkCreateDescriptorPool(VulkanApp->Device, &poolCreateInfo, nullptr, &DescriptorPool) != VK_SUCCESS)
		{
			TERMINATE_LOG("Couldn't create descriptor pool!")
			return;
		}

		LightUBO.Setup(app, vk::UboType::Dynamic, sizeof(LightDataUBO), 1);
	}

	void SceneManager::Cleanup()
	{
		LightUBO.Cleanup();

		vkDestroyDescriptorPool(VulkanApp->Device, DescriptorPool, nullptr);

		for (auto& [id, ubo] : MaterialLookupUBOs)
			ubo.Cleanup();

		for (auto& [id, ubo] : MeshLookupUBOs)
			ubo.Cleanup();

		for (auto& vboVector : MeshBuffers)
		{
			for (auto& vbo : vboVector)
				vbo.Cleanup();
		}

		for (auto& r : Renderables)
			render::CleanupRenderable(*VulkanApp, r);
	}

	void SceneManager::Update()
	{
		RM->SetActiveCamera(Cameras[ActiveCameraId]);


		//Update mesh ubos
		for (size_t i = 0; i < RegisteredMeshes.size(); ++i)
		{
			auto& findUbo = MeshLookupUBOs.find(i);
			if (findUbo == MeshLookupUBOs.end())
				continue;

			auto& mesh = RegisteredMeshes[i];
			auto& meshTransform = mesh.get().Transform;

			glm::mat4 transform(1.0f);
			transform = glm::rotate(transform, meshTransform.Rotation.w, glm::vec3(meshTransform.Rotation));
			transform = glm::scale(transform, meshTransform.Scale);
			transform = glm::translate(transform, meshTransform.Translate);

			findUbo->second.Update(&transform, 1);
		}

		//Update material ubos
		for (size_t i = 0; i < RegisteredMeshes.size(); ++i)
		{
			auto& findUbo = MaterialLookupUBOs.find(i);
			if (findUbo == MaterialLookupUBOs.end())
				continue;

			auto& mesh = RegisteredMeshes[i];
			auto& material = mesh.get().Material;
			
			findUbo->second.Update(material->GetMaterialData(), 1);
		}


		//Light ubo update

		LightDataUBO lightData;

		for (size_t i = 0; i < RegisteredPointLights.size(); ++i)
		{
			auto& l = RegisteredPointLights[i].get();

			lightData.PointLights[i].Position = { l.Position, 1.0f };
			lightData.PointLights[i].Color = { l.Color, 1.0f };
		}

		for (size_t i = 0; i < RegisteredSpotlights.size(); ++i)
		{
			auto& l = RegisteredSpotlights[i].get();

			lightData.Spotlights[i].Position = { l.Position, 1.0f };
			lightData.Spotlights[i].Direction = { l.Direction, 0.0f };
			lightData.Spotlights[i].Color = { l.Color, 1.0f };
			lightData.Spotlights[i].OuterAngle = l.OuterAngle;
			lightData.Spotlights[i].InnerAngle = l.InnerAngle;
		}

		lightData.PointLightsCount = RegisteredPointLights.size();
		lightData.SpotlightsCount = RegisteredSpotlights.size();

		LightUBO.Update(&lightData, 1);
	}

	std::vector<vk::UboDescriptor> SceneManager::CreateDescriptors(const render::BaseMaterial& material, const vk::Shader& shader)
	{
		auto reflectMap = shader.GetReflectMap();

		std::vector<vk::UboDescriptor> descriptors;

		vk::UboDescriptor globalUboDescriptor;
		vk::UboDescriptor meshUboDescriptor;
		vk::UboDescriptor materialUboDescriptor;

		{
			auto& findShaderInfo = reflectMap.find(VK_SHADER_STAGE_VERTEX_BIT);
			if (findShaderInfo == reflectMap.end())
				return {};

			for (auto d : findShaderInfo->second.DescriptorSets)
			{
				switch (d.SetId)
				{
				case ShaderDescriptorSetGlobalUBO:
					{
						for (auto& b : d.Bindings)
						{
							if(b.BindId == ShaderDescriptorBindCameraUBO)
								globalUboDescriptor.LinkUBO(RM->GlobalUBO, ShaderDescriptorBindCameraUBO);
						}
					} break;
				case ShaderDescriptorSetMeshUBO:
					{
						vk::UniformBuffer meshUBO;
						meshUBO.Setup(*VulkanApp, vk::UboType::Dynamic, sizeof(MeshUBO), 1);

						meshUboDescriptor.LinkUBO(meshUBO, 0);

						MeshLookupUBOs[RegisteredMeshes.size() - 1] = meshUBO;
					} break;
				}
			}
		}

		{
			auto& findShaderInfo = reflectMap.find(VK_SHADER_STAGE_FRAGMENT_BIT);
			if (findShaderInfo == reflectMap.end())
				return {};

			for (auto d : findShaderInfo->second.DescriptorSets)
			{
				switch (d.SetId)
				{
				case ShaderDescriptorSetGlobalUBO:
					{
						for (auto& b : d.Bindings)
						{
							if (b.BindId == ShaderDescriptorBindLightUBO)
								globalUboDescriptor.LinkUBO(LightUBO, ShaderDescriptorBindLightUBO);
						}
					} break;
				case ShaderDescriptorSetMaterialUBO:
					{
						vk::UniformBuffer materialUBO;
						materialUBO.Setup(*VulkanApp, vk::UboType::Dynamic, material.GetMaterialInfoStride(), 1);

						materialUboDescriptor.LinkUBO(materialUBO, 0);

						MaterialLookupUBOs[RegisteredMeshes.size() - 1] = materialUBO;
					} break;
				}
			}
		}

		globalUboDescriptor.Create(*VulkanApp, DescriptorPool);
		meshUboDescriptor.Create(*VulkanApp, DescriptorPool);
		materialUboDescriptor.Create(*VulkanApp, DescriptorPool);

		descriptors.push_back(globalUboDescriptor);
		descriptors.push_back(meshUboDescriptor);
		descriptors.push_back(materialUboDescriptor);

		return descriptors;
	}

	void SceneManager::SetupBuffers(render::Mesh& mesh, vk::Shader& shader, render::Renderable& renderable)
	{
		auto reflectMap = shader.GetReflectMap();

		auto& findShaderInfo = reflectMap.find(VK_SHADER_STAGE_VERTEX_BIT);
		if (findShaderInfo == reflectMap.end())
			return;

		std::vector<vk::Buffer> buffers;

		size_t bindId = 0;

		for (auto i : findShaderInfo->second.Inputs)
		{
			switch (i.LocationId)
			{
			case ShaderInputPositionLocation:
				{
					vk::Buffer positionBuffer;
					positionBuffer.Setup(*VulkanApp, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, sizeof(mesh.MeshInfo.Positions[0]), mesh.MeshInfo.Positions.size());
					positionBuffer.Update(&mesh.MeshInfo.Positions[0], mesh.MeshInfo.Positions.size());

					renderable.PositionsCount = positionBuffer.GetElementsCount();

					shader.AddInputBuffer(VK_FORMAT_R32G32B32_SFLOAT, bindId, ShaderInputPositionLocation, 0, positionBuffer.GetStride());

					buffers.push_back(positionBuffer);
				} break;
			case ShaderInputNormalLocation:
				{
					vk::Buffer normalBuffer;
					normalBuffer.Setup(*VulkanApp, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, sizeof(mesh.MeshInfo.Normals[0]), mesh.MeshInfo.Normals.size());
					normalBuffer.Update(&mesh.MeshInfo.Normals[0], mesh.MeshInfo.Normals.size());

					shader.AddInputBuffer(VK_FORMAT_R32G32B32_SFLOAT, bindId, ShaderInputNormalLocation, 0, normalBuffer.GetStride());

					buffers.push_back(normalBuffer);
				} break;
			}

			++bindId;
		}

		for (const auto& vbo : buffers)
			renderable.Buffers.push_back(vbo.GetHandler());

		MeshBuffers.push_back(buffers);
	}

	void SceneManager::RegisterMesh(render::Mesh& mesh)
	{
		if (!mesh.Material)
		{
			LOG("Couldn't register mesh without material!")
			return;
		}


		RegisteredMeshes.push_back(mesh);

		render::Renderable renderable;

		auto shader = mesh.Material->CreateShader(*VulkanApp);


		SetupBuffers(mesh, shader, renderable);


		auto descriptors = CreateDescriptors(*mesh.Material, shader);

		renderable.Descriptors = descriptors;

		std::vector<VkDescriptorSetLayout> layouts;
		for (auto& d : descriptors)
			layouts.push_back(d.GetDescriptorInfo().DescriptorSetLayout);

		if (!CreatePipeline(layouts, renderable.GraphicsPipeline, renderable.GraphicsPipelineLayout, shader))
		{
			LOG("Couldn't create pipeline for new material")
			return;
		}

		Renderables.push_back(renderable);

		shader.Cleanup();
	}
}