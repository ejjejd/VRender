#include "render_manager.h"

#include <set>
#include <fstream>

namespace manager
{
	struct CameraUboInfo
	{
		glm::mat4 ToCamera;
		glm::mat4 ToClip;
		glm::vec4 CameraPosition;
	};

	struct MeshUBO
	{
		glm::mat4 Transform;
	};

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

	graphics::Texture TextureManager::GetOrCreate(const render::MaterialTexture& texture)
	{
		auto& findRes = TexturesLookup.find(texture.ImageId);
		if (findRes == TexturesLookup.end())
		{
			graphics::Texture t;

			if (!AM->IsImageLoaded(texture.ImageId))
			{
				char pixels[] = { -1, -1, -1, -1 };
				t.Setup(*App, 1, 1, render::CreateInfoMapTextureParams());
				t.Update(pixels);
			}
			else
			{
				auto image = AM->GetImageInfo(texture.ImageId);

				t.Setup(*App, image.Width, image.Height, texture.TextureParams);
				t.Update(image.PixelsData.data());
			}

			TexturesLookup[texture.ImageId] = t;

			return t;
		}

		return findRes->second;
	}

	bool RenderManager::SetupRenderPassases()
	{
		{
			VkAttachmentDescription depthAttachment{};
			depthAttachment.format = VulkanApp->DepthFormat;
			depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
			depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
			depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
			depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
			depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
			depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
			depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

			VkAttachmentReference depthAttachmentRef{};
			depthAttachmentRef.attachment = 1;
			depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

			VkAttachmentDescription colorAttachment{};
			colorAttachment.format = VulkanApp->SwapChainFormat;
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
			subpass.pDepthStencilAttachment = &depthAttachmentRef;

			VkSubpassDependency dependency{};
			dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
			dependency.dstSubpass = 0;
			dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
			dependency.srcAccessMask = 0;
			dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
			dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

			std::vector<VkAttachmentDescription> attachments = { colorAttachment, depthAttachment };
			std::vector<VkSubpassDescription> subpasses = { subpass };
			std::vector<VkSubpassDependency> dependencies = { dependency };

			auto rpCreateRes = vk::CreateRenderPass(*VulkanApp, attachments, subpasses, dependencies);
			if (!rpCreateRes)
				return false;
			MainRenderPass = *rpCreateRes;

			Framebuffers.resize(VulkanApp->SwapChainImageViews.size());

			for (size_t i = 0; i < VulkanApp->SwapChainImageViews.size(); ++i)
			{
				std::vector<VkImageView> attachments;
				attachments.push_back(VulkanApp->SwapChainImageViews[i]);
				attachments.push_back(VulkanApp->DepthImageView);

				auto fboRes = vk::CreateFramebuffer(*VulkanApp, MainRenderPass, attachments,
					VulkanApp->SwapChainExtent.width, VulkanApp->SwapChainExtent.height);
				if (!fboRes)
					return false;

				Framebuffers[i] = *fboRes;
			}
		}

		return true;
	}
	
	std::optional<vk::Pipeline> RenderManager::CreateMeshPipeline(vk::Shader& shader, 
																	   const std::vector<VkDescriptorSetLayout>& layouts)
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

		auto pipelineRes = vk::CreateGraphicsPipeline(*VulkanApp, MainRenderPass, shader, layouts,
													  inputAssembly, viewportState, rasterizer, 
													  multisample, colorBlending, depthState);
		if (!pipelineRes)
			return std::nullopt;

		return pipelineRes;
	}

	void RenderManager::UpdateGlobalUBO()
	{
		CameraUboInfo ubo;
		ubo.ToCamera = ActiveCamera.GetViewMatrix();
		ubo.ToClip = ActiveCamera.GetProjection();
		ubo.CameraPosition = { ActiveCamera.Position, 1.0f };

		GlobalUBO.Update(&ubo, 1);
	}

	bool RenderManager::Setup(vk::VulkanApp& app, AssetManager& am)
	{
		VulkanApp = &app;

		if (!SetupRenderPassases())
			return false;

		CommandBuffers.resize(Framebuffers.size());

		VkCommandBufferAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocInfo.commandPool = VulkanApp->CommandPoolGQ;
		allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		allocInfo.commandBufferCount = CommandBuffers.size();

		if (vkAllocateCommandBuffers(VulkanApp->Device, &allocInfo, &CommandBuffers[0]) != VK_SUCCESS)
			return false;


		VkSemaphoreCreateInfo semaphoreInfo{};
		semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

		if (vkCreateSemaphore(app.Device, &semaphoreInfo, nullptr, &ImageAvailableSemaphore) != VK_SUCCESS
			|| vkCreateSemaphore(app.Device, &semaphoreInfo, nullptr, &RenderFinishedSemaphore) != VK_SUCCESS)
		{
			return false;
		}


		//Setup ubo's
		GlobalUBO.Setup(app, vk::UboType::Dynamic, sizeof(CameraUboInfo), 1);

		TM.Setup(app, am);

		VkDescriptorPoolSize poolSize{};
		poolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		poolSize.descriptorCount = Framebuffers.size() * 255;

		VkDescriptorPoolCreateInfo poolCreateInfo{};
		poolCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		poolCreateInfo.poolSizeCount = 1;
		poolCreateInfo.pPoolSizes = &poolSize;
		poolCreateInfo.maxSets = 255;

		if (vkCreateDescriptorPool(VulkanApp->Device, &poolCreateInfo, nullptr, &DescriptorPool) != VK_SUCCESS)
		{
			TERMINATE_LOG("Couldn't create descriptor pool!")
			return false;
		}


		VkDescriptorPoolSize imagePoolSize{};
		imagePoolSize.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		imagePoolSize.descriptorCount = Framebuffers.size() * 255;

		VkDescriptorPoolCreateInfo imagePoolInfo{};
		imagePoolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		imagePoolInfo.poolSizeCount = 1;
		imagePoolInfo.pPoolSizes = &imagePoolSize;
		imagePoolInfo.maxSets = 255;

		if (vkCreateDescriptorPool(VulkanApp->Device, &imagePoolInfo, nullptr, &DescriptorPoolImage) != VK_SUCCESS)
		{
			TERMINATE_LOG("Couldn't create descriptor pool!")
			return false;
		}

		LightUBO.Setup(app, vk::UboType::Dynamic, sizeof(LightDataUBO), 1);

		return true;
	}

	void RenderManager::Cleanup()
	{
		TM.Cleanup();

		LightUBO.Cleanup();

		vkDestroyDescriptorPool(VulkanApp->Device, DescriptorPool, nullptr);
		vkDestroyDescriptorPool(VulkanApp->Device, DescriptorPoolImage, nullptr);

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

		GlobalUBO.Cleanup();

		vkFreeCommandBuffers(VulkanApp->Device, VulkanApp->CommandPoolGQ, CommandBuffers.size(), &CommandBuffers[0]);

		for (size_t i = 0; i < Framebuffers.size(); i++)
			vkDestroyFramebuffer(VulkanApp->Device, Framebuffers[i], nullptr);

		vkDestroyRenderPass(VulkanApp->Device, MainRenderPass, nullptr);

		vkDestroySemaphore(VulkanApp->Device, ImageAvailableSemaphore, nullptr);
		vkDestroySemaphore(VulkanApp->Device, RenderFinishedSemaphore, nullptr);
	}

	void RenderManager::UpdateMeshUBO(const std::vector<std::reference_wrapper<render::Mesh>>& meshes)
	{
		for (size_t i = 0; i < meshes.size(); ++i)
		{
			auto& findUbo = MeshLookupUBOs.find(i);
			if (findUbo == MeshLookupUBOs.end())
				continue;

			auto& mesh = meshes[i];
			auto& meshTransform = mesh.get().Transform;

			glm::mat4 transform(1.0f);
			transform = glm::rotate(transform, meshTransform.Rotation.w, glm::vec3(meshTransform.Rotation));
			transform = glm::scale(transform, meshTransform.Scale);
			transform = glm::translate(transform, meshTransform.Translate);

			findUbo->second.Update(&transform, 1);
		}

		for (size_t i = 0; i < meshes.size(); ++i)
		{
			auto& findUbo = MaterialLookupUBOs.find(i);
			if (findUbo == MaterialLookupUBOs.end())
				continue;

			auto& mesh = meshes[i];
			auto& material = mesh.get().Material;

			findUbo->second.Update(material->GetMaterialData(), 1);
		}

	}

	void RenderManager::UpdateLightUBO(const std::vector<std::reference_wrapper<render::PointLight>>& pointLights,
									   const std::vector<std::reference_wrapper<render::Spotlight>>& spotlights)
	{
		LightDataUBO lightData;

		for (size_t i = 0; i < pointLights.size(); ++i)
		{
			auto& l = pointLights[i].get();

			lightData.PointLights[i].Position = { l.Position, 1.0f };
			lightData.PointLights[i].Color = { l.Color, 1.0f };
		}

		for (size_t i = 0; i < spotlights.size(); ++i)
		{
			auto& l = spotlights[i].get();

			lightData.Spotlights[i].Position = { l.Position, 1.0f };
			lightData.Spotlights[i].Direction = { l.Direction, 0.0f };
			lightData.Spotlights[i].Color = { l.Color, 1.0f };
			lightData.Spotlights[i].OuterAngle = l.OuterAngle;
			lightData.Spotlights[i].InnerAngle = l.InnerAngle;
		}

		lightData.PointLightsCount = pointLights.size();
		lightData.SpotlightsCount = spotlights.size();

		LightUBO.Update(&lightData, 1);
	}

	void RenderManager::Update()
	{
		UpdateGlobalUBO();

		for (size_t i = 0; i < CommandBuffers.size(); ++i)
		{
			VkCommandBufferBeginInfo beginInfo{};
			beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
			beginInfo.flags = 0;
			beginInfo.pInheritanceInfo = nullptr;

			if (vkBeginCommandBuffer(CommandBuffers[i], &beginInfo) != VK_SUCCESS)
				return;

			VkRenderPassBeginInfo renderPassInfo{};
			renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
			renderPassInfo.renderPass = MainRenderPass;
			renderPassInfo.framebuffer = Framebuffers[i];
			renderPassInfo.renderArea.offset = { 0, 0 };
			renderPassInfo.renderArea.extent = VulkanApp->SwapChainExtent;

			VkClearValue clearColor = { 0.0f, 0.0f, 0.0f, 1.0f };

			VkClearValue clearDepth;
			clearDepth.depthStencil.depth = 1.0f;

			VkClearValue clearValues[2] = { clearColor, clearDepth };

			renderPassInfo.clearValueCount = 2;
			renderPassInfo.pClearValues = &clearValues[0];

			vkCmdBeginRenderPass(CommandBuffers[i], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

			for (const auto& r : Renderables)
			{
				vkCmdBindPipeline(CommandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, r.GraphicsPipeline);

				std::vector<VkDeviceSize> offsets(r.Buffers.size(), 0);

				vkCmdBindVertexBuffers(CommandBuffers[i], 0, r.Buffers.size(), r.Buffers.data(), offsets.data());


				std::vector<VkDescriptorSet> descriptors;

				for (auto d : r.Descriptors)
				{
					auto descriptorSets = d.DescriptorSets;

					if (descriptorSets.size() == Framebuffers.size())
						descriptors.push_back(descriptorSets[i]);
					else if (descriptorSets.size() == 1)
						descriptors.push_back(descriptorSets[0]);
					else
						TERMINATE_LOG("Invalid descriptor created!")
				}

				vkCmdBindDescriptorSets(CommandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, r.GraphicsPipelineLayout, 0, descriptors.size(), descriptors.data(), 0, nullptr);


				vkCmdDraw(CommandBuffers[i], r.PositionsCount, 1, 0, 0);
			}

			vkCmdEndRenderPass(CommandBuffers[i]);

			if (vkEndCommandBuffer(CommandBuffers[i]) != VK_SUCCESS)
				return;
		}

		uint32_t imageId = 0;
		VkResult acqResult = vkAcquireNextImageKHR(VulkanApp->Device, VulkanApp->SwapChain, UINT64_MAX, ImageAvailableSemaphore, VK_NULL_HANDLE, &imageId);


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

		if (vkQueueSubmit(VulkanApp->GraphicsQueue, 1, &submitInfo, VK_NULL_HANDLE) != VK_SUCCESS)
			return;

		VkPresentInfoKHR presentInfo{};
		presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
		presentInfo.waitSemaphoreCount = 1;
		presentInfo.pWaitSemaphores = signalSemaphores;

		VkSwapchainKHR swapChains[] = { VulkanApp->SwapChain };
		presentInfo.swapchainCount = 1;
		presentInfo.pSwapchains = swapChains;
		presentInfo.pImageIndices = &imageId;

		vkQueuePresentKHR(VulkanApp->PresentQueue, &presentInfo);
		vkQueueWaitIdle(VulkanApp->PresentQueue);
	}

	std::vector<vk::Descriptor> RenderManager::SetupMeshDescriptors(const render::BaseMaterial& material, const vk::Shader& shader, const size_t meshId)
	{
		auto reflectMap = shader.GetReflectMap();

		std::vector<vk::Descriptor> descriptors;

		vk::UboDescriptor globalUboDescriptor;
		vk::UboDescriptor meshUboDescriptor;
		vk::UboDescriptor materialUboDescriptor;

		vk::TextureDescriptor materialTexturesDescriptor;

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
						if (b.BindId == ShaderDescriptorBindCameraUBO)
							globalUboDescriptor.LinkUBO(GlobalUBO, ShaderDescriptorBindCameraUBO);
					}
				} break;
				case ShaderDescriptorSetMeshUBO:
				{
					vk::UniformBuffer meshUBO;
					meshUBO.Setup(*VulkanApp, vk::UboType::Dynamic, sizeof(MeshUBO), 1);

					meshUboDescriptor.LinkUBO(meshUBO, 0);

					MeshLookupUBOs[meshId] = meshUBO;
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

					MaterialLookupUBOs[meshId] = materialUBO;
				} break;
				case ShaderDescriptorSetMaterialTextures:
				{
					for (auto& b : d.Bindings)
					{
						auto texture = TM.GetOrCreate(material.GetMaterialTexturesIds()[b.BindId]);
						materialTexturesDescriptor.LinkTexture(texture, b.BindId);
					}
				} break;
				}
			}
		}

		globalUboDescriptor.Create(*VulkanApp, DescriptorPool);
		meshUboDescriptor.Create(*VulkanApp, DescriptorPool);
		materialUboDescriptor.Create(*VulkanApp, DescriptorPool);

		materialTexturesDescriptor.Create(*VulkanApp, DescriptorPoolImage);

		descriptors.push_back(globalUboDescriptor.GetDescriptorInfo());
		descriptors.push_back(meshUboDescriptor.GetDescriptorInfo());
		descriptors.push_back(materialUboDescriptor.GetDescriptorInfo());
		descriptors.push_back(materialTexturesDescriptor.GetDescriptorInfo());

		return descriptors;
	}

	void RenderManager::SetupMeshBuffers(const render::Mesh& mesh, vk::Shader& shader, render::Renderable& renderable)
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
				positionBuffer.Update((void*)mesh.MeshInfo.Positions.data(), mesh.MeshInfo.Positions.size());

				renderable.PositionsCount = positionBuffer.GetElementsCount();

				shader.AddInputBuffer(VK_FORMAT_R32G32B32_SFLOAT, bindId, ShaderInputPositionLocation, 0, positionBuffer.GetStride());

				buffers.push_back(positionBuffer);
			} break;
			case ShaderInputNormalLocation:
			{
				vk::Buffer normalBuffer;
				normalBuffer.Setup(*VulkanApp, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, sizeof(mesh.MeshInfo.Normals[0]), mesh.MeshInfo.Normals.size());
				normalBuffer.Update((void*)mesh.MeshInfo.Normals.data(), mesh.MeshInfo.Normals.size());

				shader.AddInputBuffer(VK_FORMAT_R32G32B32_SFLOAT, bindId, ShaderInputNormalLocation, 0, normalBuffer.GetStride());

				buffers.push_back(normalBuffer);
			} break;
			case ShaderInputUvLocation:
			{
				vk::Buffer uvBuffer;
				uvBuffer.Setup(*VulkanApp, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, sizeof(mesh.MeshInfo.UVs[0]), mesh.MeshInfo.UVs.size());
				uvBuffer.Update((void*)mesh.MeshInfo.UVs.data(), mesh.MeshInfo.UVs.size());

				shader.AddInputBuffer(VK_FORMAT_R32G32B32_SFLOAT, bindId, ShaderInputUvLocation, 0, uvBuffer.GetStride());

				buffers.push_back(uvBuffer);
			} break;
			case ShaderInputTangentLocation:
			{
				vk::Buffer tangentBuffer;
				tangentBuffer.Setup(*VulkanApp, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, sizeof(mesh.MeshInfo.Tangents[0]), mesh.MeshInfo.Tangents.size());
				tangentBuffer.Update((void*)mesh.MeshInfo.Tangents.data(), mesh.MeshInfo.Tangents.size());

				shader.AddInputBuffer(VK_FORMAT_R32G32B32_SFLOAT, bindId, ShaderInputTangentLocation, 0, tangentBuffer.GetStride());

				buffers.push_back(tangentBuffer);
			} break;
			case ShaderInputBitangentLocation:
			{
				vk::Buffer bitangentBuffer;
				bitangentBuffer.Setup(*VulkanApp, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, sizeof(mesh.MeshInfo.Bitangents[0]), mesh.MeshInfo.Bitangents.size());
				bitangentBuffer.Update((void*)mesh.MeshInfo.Bitangents.data(), mesh.MeshInfo.Bitangents.size());

				shader.AddInputBuffer(VK_FORMAT_R32G32B32_SFLOAT, bindId, ShaderInputBitangentLocation, 0, bitangentBuffer.GetStride());

				buffers.push_back(bitangentBuffer);
			} break;
			}

			++bindId;
		}

		for (const auto& vbo : buffers)
			renderable.Buffers.push_back(vbo.GetHandler());

		MeshBuffers.push_back(buffers);
	}

	void RenderManager::RegisterMesh(const render::Mesh& mesh, const size_t meshId)
	{
		if (!mesh.Material)
		{
			LOG("Couldn't register mesh without material!")
			return;
		}


		render::Renderable renderable;

		auto shader = mesh.Material->CreateShader(*VulkanApp);


		SetupMeshBuffers(mesh, shader, renderable);


		auto descriptors = SetupMeshDescriptors(*mesh.Material, shader, meshId);

		renderable.Descriptors = descriptors;

		std::vector<VkDescriptorSetLayout> layouts;
		for (auto& d : descriptors)
			layouts.push_back(d.DescriptorSetLayout);

		auto pipelineRes = CreateMeshPipeline(shader, layouts);
		if (!pipelineRes)
		{
			LOG("Couldn't create graphics pipeline for the mesh!")
			return;
		}

		renderable.GraphicsPipelineLayout = pipelineRes->Layout;
		renderable.GraphicsPipeline = pipelineRes->Handle;

		Renderables.push_back(renderable);

		shader.Cleanup();
	}
}