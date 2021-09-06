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

	void CleanupOffscreenPass(const vk::VulkanApp& app, const OffscreenPass& pass)
	{
		pass.Renderable.Descriptor.Destroy();
		vkDestroyPipelineLayout(app.Device, pass.Renderable.PipelineLayout, nullptr);
		vkDestroyPipeline(app.Device, pass.Renderable.Pipeline, nullptr);

		for (const auto& fbo : pass.Framebuffers)
			vkDestroyFramebuffer(app.Device, fbo, nullptr);

		pass.ColorTexture.Cleanup();
		pass.DepthTexture.Cleanup();

		vkDestroyRenderPass(app.Device, pass.PassHandler, nullptr);
	}

	vk::Texture TextureManager::GetOrCreate(const render::MaterialTexture& texture, const vk::DescriptorImageType type)
	{
		auto& findRes = TexturesLookup.find(texture.ImageId);
		if (findRes == TexturesLookup.end())
		{
			vk::Texture t;

			vk::TextureImageInfo imageInfo;
			imageInfo.Type = VK_IMAGE_TYPE_2D;
			imageInfo.ViewType = VK_IMAGE_VIEW_TYPE_2D;
			imageInfo.Format = VK_FORMAT_R8G8B8A8_SRGB;
			imageInfo.ViewAspect = VK_IMAGE_ASPECT_COLOR_BIT;
			imageInfo.UsageFlags = VK_IMAGE_USAGE_TRANSFER_DST_BIT 
								   | VK_IMAGE_USAGE_SAMPLED_BIT;
			imageInfo.Layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
				
			if (type == vk::DescriptorImageType::Cubemap)
			{
				imageInfo.ViewType = VK_IMAGE_VIEW_TYPE_CUBE;
				imageInfo.UsageFlags = VK_IMAGE_USAGE_SAMPLED_BIT;
				imageInfo.Layout = VK_IMAGE_LAYOUT_GENERAL;
				imageInfo.CreateFlags = VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;
			}

			if (!AM->IsImageLoaded(texture.ImageId)) 
			{
				if (type == vk::DescriptorImageType::Image2d)
				{
					t.Setup(*App, 1, 1, imageInfo, render::CreateInfoMapTextureParams());

					//Fill texture with white color if the texture view is 2d
					char pixels[] = { -1, -1, -1, -1 };
					t.Update(pixels, 4);
				}
				else if (type == vk::DescriptorImageType::Cubemap)
				{
					t.Setup(*App, 1, 1, imageInfo, render::CreateInfoMapTextureParams(), 1, 6);

					t.SetLayout(App->ComputeQueue, App->CommandPoolCQ,
								vk::layout::SetCubeImageLayoutFromComputeWriteToGraphicsShader);
				}
			}
			else
			{
				auto image = AM->GetImageInfo(texture.ImageId);

				if (image.Hdr)
					imageInfo.Format = VK_FORMAT_R32G32B32A32_SFLOAT;


				t.Setup(*App, image.Width, image.Height, imageInfo, texture.TextureParams);
				t.Update(image.PixelsData.data(), 4 * (image.Hdr ? sizeof(float) : 1));
				t.SetLayout(App->GraphicsQueue, App->CommandPoolGQ,
							vk::layout::SetImageLayoutFromTransferToGraphicsShader);
			}

			TexturesLookup[texture.ImageId] = t;

			return t;
		}

		return findRes->second;
	}

	void CleanupRenderablesInfos(const vk::VulkanApp& app, const MeshRenderablesInfos& infos)
	{
		for (const auto& descriptors : infos.Descriptors)
		{
			for(const auto& d : descriptors)
				vk::CleanupDescriptor(app, d);
		}

		for (const auto& vbos : infos.Buffers)
		{
			for (const auto& b : vbos)
				b.Cleanup();
		}

		for (const auto& l : infos.GraphicsPipelineLayouts)
			vkDestroyPipelineLayout(app.Device, l, nullptr);

		for (const auto& p : infos.GraphicsPipelines)
			vkDestroyPipeline(app.Device, p, nullptr);

		for (const auto& ubo : infos.MeshUBOs)
			ubo.Cleanup();

		for (const auto& ubo : infos.MaterialUBOs)
			ubo.Cleanup();
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

		{
			vk::TextureParams textureParams;
			textureParams.MagFilter = VK_FILTER_LINEAR;
			textureParams.MinFilter = VK_FILTER_LINEAR;
			textureParams.AddressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
			textureParams.AddressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;

			vk::TextureImageInfo imageInfo;
			imageInfo.Type = VK_IMAGE_TYPE_2D;
			imageInfo.ViewType = VK_IMAGE_VIEW_TYPE_2D;
			imageInfo.Format = VK_FORMAT_R16G16B16A16_SFLOAT;
			imageInfo.ViewAspect = VK_IMAGE_ASPECT_COLOR_BIT;
			imageInfo.UsageFlags = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT
								   | VK_IMAGE_USAGE_SAMPLED_BIT;
			imageInfo.Layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

			HdrPass.ColorTexture.Setup(*VulkanApp, VulkanApp->SwapChainExtent.width, VulkanApp->SwapChainExtent.height,
										imageInfo, textureParams);

			imageInfo.ViewAspect = VK_IMAGE_ASPECT_DEPTH_BIT;
			imageInfo.Format = VulkanApp->DepthFormat;
			imageInfo.ViewAspect = VK_IMAGE_ASPECT_DEPTH_BIT;;
			imageInfo.UsageFlags = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT
								   | VK_IMAGE_USAGE_SAMPLED_BIT;
			imageInfo.Layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

			HdrPass.DepthTexture.Setup(*VulkanApp, VulkanApp->SwapChainExtent.width, VulkanApp->SwapChainExtent.height,
									   imageInfo, textureParams);

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
			colorAttachment.format = VK_FORMAT_R16G16B16A16_SFLOAT;
			colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
			colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
			colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
			colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
			colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
			colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
			colorAttachment.finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

			VkAttachmentReference colorAttachmentRef{};
			colorAttachmentRef.attachment = 0;
			colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

			VkSubpassDescription subpass{};
			subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
			subpass.colorAttachmentCount = 1;
			subpass.pColorAttachments = &colorAttachmentRef;
			subpass.pDepthStencilAttachment = &depthAttachmentRef;


			std::vector<VkSubpassDependency> dependencies(2);

			dependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;
			dependencies[0].dstSubpass = 0;
			dependencies[0].srcStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
			dependencies[0].dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
			dependencies[0].srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
			dependencies[0].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
			dependencies[0].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

			dependencies[1].srcSubpass = 0;
			dependencies[1].dstSubpass = VK_SUBPASS_EXTERNAL;
			dependencies[1].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
			dependencies[1].dstStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
			dependencies[1].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
			dependencies[1].dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
			dependencies[1].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;


			std::vector<VkAttachmentDescription> attachments = { colorAttachment, depthAttachment };
			std::vector<VkSubpassDescription> subpasses = { subpass };

			auto rpCreateRes = vk::CreateRenderPass(*VulkanApp, attachments, subpasses, dependencies);
			if (!rpCreateRes)
				return false;
			HdrPass.PassHandler = *rpCreateRes;

			for (size_t i = 0; i < VulkanApp->SwapChainImageViews.size(); ++i)
			{
				std::vector<VkImageView> attachments;
				attachments.push_back(HdrPass.ColorTexture.GetImage().GetViewHandler());
				attachments.push_back(HdrPass.DepthTexture.GetImage().GetViewHandler());

				auto fboRes = vk::CreateFramebuffer(*VulkanApp, HdrPass.PassHandler, attachments,
													VulkanApp->SwapChainExtent.width, VulkanApp->SwapChainExtent.height);
				if (!fboRes)
					return false;

				HdrPass.Framebuffers.push_back(*fboRes);
			}
			

			auto& renderable = HdrPass.Renderable;

			vk::TextureDescriptor descriptor;
			descriptor.LinkTexture(HdrPass.ColorTexture, 0);
			descriptor.Create(*VulkanApp, DescriptorPoolImage);

			vk::Shader shader;
			shader.Setup(*VulkanApp);

			shader.AddStage("res/shaders/offscreen/hdr_vert.spv", VK_SHADER_STAGE_VERTEX_BIT);
			shader.AddStage("res/shaders/offscreen/hdr_frag.spv", VK_SHADER_STAGE_FRAGMENT_BIT);

			auto pipeline = CreateMainPipeline(shader, { descriptor.GetDescriptorInfo().DescriptorSetLayout });
			if (!pipeline)
				return false;

			renderable.Pipeline = pipeline->Handle;
			renderable.PipelineLayout = pipeline->Layout;
			renderable.Descriptor = descriptor;

			shader.Cleanup();
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
		depthState.depthCompareOp = VK_COMPARE_OP_LESS;
		depthState.depthBoundsTestEnable = VK_FALSE;
		depthState.minDepthBounds = 0.0f;
		depthState.maxDepthBounds = 1.0f;
		depthState.stencilTestEnable = VK_FALSE;


		const uint8_t dynamicStatesCount = 2;
		const VkDynamicState pipelineStates[dynamicStatesCount] =
		{
			VK_DYNAMIC_STATE_DEPTH_COMPARE_OP_EXT,
			VK_DYNAMIC_STATE_CULL_MODE_EXT
		};

		VkPipelineDynamicStateCreateInfo dynamicState{};
		dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
		dynamicState.dynamicStateCount = dynamicStatesCount;
		dynamicState.pDynamicStates = pipelineStates;

		auto pipelineRes = vk::CreateGraphicsPipeline(*VulkanApp, HdrPass.PassHandler, shader, layouts,
													  inputAssembly, viewportState, rasterizer, 
													  multisample, colorBlending, depthState, dynamicState);
		if (!pipelineRes)
			return std::nullopt;

		return pipelineRes;
	}

	std::optional<vk::Pipeline> RenderManager::CreateMainPipeline(vk::Shader& shader,
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
		rasterizer.cullMode = VK_CULL_MODE_NONE;
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

		VkPipelineDynamicStateCreateInfo dynamicState{};
		dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;

		auto pipelineRes = vk::CreateGraphicsPipeline(*VulkanApp, MainRenderPass, shader, layouts,
													  inputAssembly, viewportState, rasterizer,
													  multisample, colorBlending, depthState, dynamicState);
		if (!pipelineRes)
			return std::nullopt;

		return pipelineRes;
	}

	bool RenderManager::Setup(vk::VulkanApp& app, AssetManager& am)
	{
		VulkanApp = &app;
		AM = &am;

		VkDescriptorPoolSize poolSize{};
		poolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		poolSize.descriptorCount = app.SwapChainImages.size() * 255;

		VkDescriptorPoolCreateInfo poolCreateInfo{};
		poolCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		poolCreateInfo.poolSizeCount = 1;
		poolCreateInfo.pPoolSizes = &poolSize;
		poolCreateInfo.maxSets = 255;

		auto res = vkCreateDescriptorPool(VulkanApp->Device, &poolCreateInfo, nullptr, &DescriptorPool);
		ASSERT(res == VK_SUCCESS, "Couldn't create descriptor pool!");

		VkDescriptorPoolSize imagePoolSize{};
		imagePoolSize.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		imagePoolSize.descriptorCount = app.SwapChainImages.size() * 255;

		VkDescriptorPoolCreateInfo imagePoolInfo{};
		imagePoolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		imagePoolInfo.poolSizeCount = 1;
		imagePoolInfo.pPoolSizes = &imagePoolSize;
		imagePoolInfo.maxSets = 255;

		res = vkCreateDescriptorPool(VulkanApp->Device, &imagePoolInfo, nullptr, &DescriptorPoolImage);
		ASSERT(res == VK_SUCCESS, "Couldn't create descriptor pool!");

		VkDescriptorPoolSize imageStoragePoolSize{};
		imageStoragePoolSize.type = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
		imageStoragePoolSize.descriptorCount = app.SwapChainImages.size() * 255;

		VkDescriptorPoolCreateInfo imageStoragePoolInfo{};
		imageStoragePoolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		imageStoragePoolInfo.poolSizeCount = 1;
		imageStoragePoolInfo.pPoolSizes = &imageStoragePoolSize;
		imageStoragePoolInfo.maxSets = 255;

		res = vkCreateDescriptorPool(VulkanApp->Device, &imageStoragePoolInfo, nullptr, &DescriptorPoolImageStorage);
		ASSERT(res == VK_SUCCESS, "Couldn't create descriptor pool!");


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
		LightUBO.Setup(app, vk::UboType::Dynamic, sizeof(LightDataUBO), 1);

		TM.Setup(app, am);

		return true;
	}

	void RenderManager::Cleanup()
	{
		CleanupRenderablesInfos(*VulkanApp, RenderablesInfos);

		LightUBO.Cleanup();
		GlobalUBO.Cleanup();

		TM.Cleanup();

		vkDestroyDescriptorPool(VulkanApp->Device, DescriptorPool, nullptr);
		vkDestroyDescriptorPool(VulkanApp->Device, DescriptorPoolImage, nullptr);
		vkDestroyDescriptorPool(VulkanApp->Device, DescriptorPoolImageStorage, nullptr);

		vkFreeCommandBuffers(VulkanApp->Device, VulkanApp->CommandPoolGQ, CommandBuffers.size(), &CommandBuffers[0]);

		for (size_t i = 0; i < Framebuffers.size(); i++)
			vkDestroyFramebuffer(VulkanApp->Device, Framebuffers[i], nullptr);

		CleanupOffscreenPass(*VulkanApp, HdrPass);

		vkDestroyRenderPass(VulkanApp->Device, MainRenderPass, nullptr);

		vkDestroySemaphore(VulkanApp->Device, ImageAvailableSemaphore, nullptr);
		vkDestroySemaphore(VulkanApp->Device, RenderFinishedSemaphore, nullptr);
	}

	void RenderManager::UpdateGlobalUBO()
	{
		CameraUboInfo ubo;
		ubo.ToCamera = ActiveCamera.GetViewMatrix();
		ubo.ToClip = ActiveCamera.GetProjection();
		ubo.CameraPosition = { ActiveCamera.Position, 1.0f };

		GlobalUBO.Update(&ubo, 1);
	}

	void RenderManager::UpdateMeshUBO(const std::vector<scene::MeshRenderable*>& meshes)
	{
		for (size_t i = 0; i < meshes.size(); ++i)
		{
			if (i >= RenderablesInfos.MeshUBOs.size())
				break;

			auto& mesh = meshes[i];

			glm::mat4 transform(1.0f);
			transform = glm::rotate(transform, mesh->GetWorldRotation().w, glm::vec3(mesh->GetWorldRotation()));
			transform = glm::scale(transform, mesh->GetWorldScale());
			transform = glm::translate(transform, mesh->GetWorldPosition());

			RenderablesInfos.MeshUBOs[i].Update(&transform, 1);
		}

		for (size_t i = 0; i < meshes.size(); ++i)
		{
			if (i >= RenderablesInfos.MaterialUBOs.size())
				break;

			auto& mesh = meshes[i];
			auto& material = mesh->Material;

			RenderablesInfos.MaterialUBOs[i].Update(material->GetMaterialData(), 1);
		}

	}

	void RenderManager::UpdateLightUBO(const std::vector<scene::PointLight*>& pointLights,
									   const std::vector<scene::Spotlight*>& spotlights)
	{
		LightDataUBO lightData;

		for (size_t i = 0; i < pointLights.size(); ++i)
		{
			auto& l = pointLights[i];

			lightData.PointLights[i].Position = { l->Position, 1.0f };
			lightData.PointLights[i].Color = { l->Color, 1.0f };
		}

		for (size_t i = 0; i < spotlights.size(); ++i)
		{
			auto& l = spotlights[i];

			lightData.Spotlights[i].Position = { l->Position, 1.0f };
			lightData.Spotlights[i].Direction = { glm::vec3(l->GetWorldRotation()), 0.0f };
			lightData.Spotlights[i].Color = { l->Color, 1.0f };
			lightData.Spotlights[i].OuterAngle = l->OuterAngle;
			lightData.Spotlights[i].InnerAngle = l->InnerAngle;
		}

		lightData.PointLightsCount = pointLights.size();
		lightData.SpotlightsCount = spotlights.size();

		LightUBO.Update(&lightData, 1);
	}

	void RenderManager::Draw(const uint8_t imageId)
	{
		for (size_t j = 0; j < RenderablesInfos.GraphicsPipelines.size(); ++j)
		{
			vkCmdBindPipeline(CommandBuffers[imageId], VK_PIPELINE_BIND_POINT_GRAPHICS, RenderablesInfos.GraphicsPipelines[j]);


			std::vector<VkBuffer> buffers;
			for (const auto& b : RenderablesInfos.Buffers[j])
				buffers.push_back(b.GetHandler());

			std::vector<VkDeviceSize> offsets(buffers.size(), 0);

			vkCmdBindVertexBuffers(CommandBuffers[imageId], 0, buffers.size(), buffers.data(), offsets.data());


			std::vector<VkDescriptorSet> descriptors;

			for (auto d : RenderablesInfos.Descriptors[j])
			{
				auto descriptorSets = d.DescriptorSets;

				ASSERT(descriptorSets.size() != 0, "Invalid descriptor created!")

				if (descriptorSets.size() == Framebuffers.size())
					descriptors.push_back(descriptorSets[imageId]);
				else
					descriptors.push_back(descriptorSets[0]);
			}

			vkCmdBindDescriptorSets(CommandBuffers[imageId], VK_PIPELINE_BIND_POINT_GRAPHICS, RenderablesInfos.GraphicsPipelineLayouts[j],
									0, descriptors.size(), descriptors.data(), 0, nullptr);


			//Set dynamic states values
			vk::CmdSetDepthOp(*VulkanApp, CommandBuffers[imageId], RenderablesInfos.AdditionalInfo[j].DepthCompareOp);
			vk::CmdSetCullMode(*VulkanApp, CommandBuffers[imageId], RenderablesInfos.AdditionalInfo[j].FacesCullMode);


			vkCmdDraw(CommandBuffers[imageId], RenderablesInfos.Buffers[j][0].GetElementsCount(), 1, 0, 0);
		}
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
			renderPassInfo.renderPass = HdrPass.PassHandler;
			renderPassInfo.framebuffer = HdrPass.Framebuffers[i];
			renderPassInfo.renderArea.offset = { 0, 0 };
			renderPassInfo.renderArea.extent = VulkanApp->SwapChainExtent;

			VkClearValue clearColor = { 0.0f, 0.0f, 0.0f, 1.0f };

			VkClearValue clearDepth;
			clearDepth.depthStencil.depth = 1.0f;

			VkClearValue clearValues[2] = { clearColor, clearDepth };

			renderPassInfo.clearValueCount = 2;
			renderPassInfo.pClearValues = &clearValues[0];

			vkCmdBeginRenderPass(CommandBuffers[i], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

			Draw(i);

			vkCmdEndRenderPass(CommandBuffers[i]);


			//Render fullscreen quad
			renderPassInfo.renderPass = MainRenderPass;
			renderPassInfo.framebuffer = Framebuffers[i];
			vkCmdBeginRenderPass(CommandBuffers[i], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

			vkCmdBindPipeline(CommandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, HdrPass.Renderable.Pipeline);

			vkCmdBindDescriptorSets(CommandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, HdrPass.Renderable.PipelineLayout,
				0, 1, HdrPass.Renderable.Descriptor.GetDescriptorInfo().DescriptorSets.data(), 0, nullptr);

			vkCmdDraw(CommandBuffers[i], 6, 1, 0, 0);

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

	std::vector<vk::Descriptor> RenderManager::SetupMeshDescriptors(const render::BaseMaterial& material, const vk::Shader& shader)
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
						meshUBO.Setup(*VulkanApp, vk::UboType::Static, sizeof(MeshUBO), 1);

						meshUboDescriptor.LinkUBO(meshUBO, 0);

						RenderablesInfos.MeshUBOs.push_back(meshUBO);
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
						materialUBO.Setup(*VulkanApp, vk::UboType::Static, material.GetMaterialInfoStride(), 1);

						materialUboDescriptor.LinkUBO(materialUBO, 0);

						RenderablesInfos.MaterialUBOs.push_back(materialUBO);
					} break;
				case ShaderDescriptorSetMaterialTextures:
					{
						for (auto& b : d.Bindings)
						{
							auto texAccess = material.GetMaterialTexturesIds()[b.BindId];
							auto texture = TM.GetOrCreate(texAccess, b.ImageType);

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

	std::vector<vk::Buffer> RenderManager::SetupMeshBuffers(const scene::MeshRenderable* mesh, vk::Shader& shader)
	{
		auto reflectMap = shader.GetReflectMap();

		auto& findShaderInfo = reflectMap.find(VK_SHADER_STAGE_VERTEX_BIT);
		if (findShaderInfo == reflectMap.end())
			return {};

		std::vector<vk::Buffer> buffers;

		size_t bindId = 0;

		for (auto i : findShaderInfo->second.Inputs)
		{
			switch (i.LocationId)
			{
			case ShaderInputPositionLocation:
				{
					vk::Buffer positionBuffer;
					positionBuffer.Setup(*VulkanApp, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, sizeof(mesh->Info.Positions[0]), mesh->Info.Positions.size());
					positionBuffer.Update((void*)mesh->Info.Positions.data(), mesh->Info.Positions.size());

					shader.AddInputBuffer(VK_FORMAT_R32G32B32_SFLOAT, bindId, ShaderInputPositionLocation, 0, positionBuffer.GetStride());

					buffers.push_back(positionBuffer);
				} break;
			case ShaderInputNormalLocation:
				{
					vk::Buffer normalBuffer;
					normalBuffer.Setup(*VulkanApp, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, sizeof(mesh->Info.Normals[0]), mesh->Info.Normals.size());
					normalBuffer.Update((void*)mesh->Info.Normals.data(), mesh->Info.Normals.size());

					shader.AddInputBuffer(VK_FORMAT_R32G32B32_SFLOAT, bindId, ShaderInputNormalLocation, 0, normalBuffer.GetStride());

					buffers.push_back(normalBuffer);
				} break;
			case ShaderInputUvLocation:
				{
					vk::Buffer uvBuffer;
					uvBuffer.Setup(*VulkanApp, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, sizeof(mesh->Info.UVs[0]), mesh->Info.UVs.size());
					uvBuffer.Update((void*)mesh->Info.UVs.data(), mesh->Info.UVs.size());

					shader.AddInputBuffer(VK_FORMAT_R32G32B32_SFLOAT, bindId, ShaderInputUvLocation, 0, uvBuffer.GetStride());

					buffers.push_back(uvBuffer);
				} break;
			case ShaderInputTangentLocation:
				{
					vk::Buffer tangentBuffer;
					tangentBuffer.Setup(*VulkanApp, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, sizeof(mesh->Info.Tangents[0]), mesh->Info.Tangents.size());
					tangentBuffer.Update((void*)mesh->Info.Tangents.data(), mesh->Info.Tangents.size());

					shader.AddInputBuffer(VK_FORMAT_R32G32B32_SFLOAT, bindId, ShaderInputTangentLocation, 0, tangentBuffer.GetStride());

					buffers.push_back(tangentBuffer);
				} break;
			case ShaderInputBitangentLocation:
				{
					vk::Buffer bitangentBuffer;
					bitangentBuffer.Setup(*VulkanApp, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, sizeof(mesh->Info.Bitangents[0]), mesh->Info.Bitangents.size());
					bitangentBuffer.Update((void*)mesh->Info.Bitangents.data(), mesh->Info.Bitangents.size());

					shader.AddInputBuffer(VK_FORMAT_R32G32B32_SFLOAT, bindId, ShaderInputBitangentLocation, 0, bitangentBuffer.GetStride());

					buffers.push_back(bitangentBuffer);
				} break;
			}

			++bindId;
		}

		return buffers;
	}

	void RenderManager::RegisterMesh(scene::MeshRenderable* mesh)
	{
		if (!mesh->Material)
		{
			LOGE("Couldn't register mesh without material!");
			return;
		}


		auto shader = mesh->Material->CreateShader(*VulkanApp);

		auto buffers = SetupMeshBuffers(mesh, shader);

		auto descriptors = SetupMeshDescriptors(*mesh->Material, shader);

		std::vector<VkDescriptorSetLayout> layouts;
		for (auto& d : descriptors)
			layouts.push_back(d.DescriptorSetLayout);

		auto pipelineRes = CreateMeshPipeline(shader, layouts);
		if (!pipelineRes)
		{
			LOGE("Couldn't create graphics pipeline for the mesh!");
			return;
		}


		RenderablesInfos.GraphicsPipelineLayouts.push_back(pipelineRes->Layout);
		RenderablesInfos.GraphicsPipelines.push_back(pipelineRes->Handle);
		RenderablesInfos.AdditionalInfo.push_back(mesh->Render);
		RenderablesInfos.Buffers.push_back(buffers);
		RenderablesInfos.Descriptors.push_back(descriptors);

		shader.Cleanup();
	}

	size_t RenderManager::GenerateCubemapFromHDR(const asset::AssetId id, const uint16_t resolution)
	{
		vk::TextureParams params;
		params.MagFilter = VK_FILTER_LINEAR;
		params.MinFilter = VK_FILTER_LINEAR;
		params.AddressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
		params.AddressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;

		vk::TextureImageInfo hdrImageInfo;
		hdrImageInfo.Type = VK_IMAGE_TYPE_2D;
		hdrImageInfo.Format = VK_FORMAT_R32G32B32A32_SFLOAT;
		hdrImageInfo.ViewType = VK_IMAGE_VIEW_TYPE_2D;
		hdrImageInfo.ViewAspect = VK_IMAGE_ASPECT_COLOR_BIT;
		hdrImageInfo.UsageFlags = VK_IMAGE_USAGE_TRANSFER_DST_BIT
								  | VK_IMAGE_USAGE_SAMPLED_BIT
								  | VK_IMAGE_USAGE_STORAGE_BIT;
		hdrImageInfo.Layout = VK_IMAGE_LAYOUT_GENERAL;

		auto hdrData = AM->GetImageInfo(id);
		vk::Texture hdrTexture;
		hdrTexture.Setup(*VulkanApp, hdrData.Width, hdrData.Height, hdrImageInfo, params);
		hdrTexture.Update(hdrData.PixelsData.data(), 4 * sizeof(float));

		hdrTexture.SetLayout(VulkanApp->ComputeQueue, VulkanApp->CommandPoolCQ,
							 vk::layout::SetImageLayoutFromTransferToComputeRead);

		vk::TextureImageInfo cubemapImageInfo;
		cubemapImageInfo.Type = VK_IMAGE_TYPE_2D;
		cubemapImageInfo.Format = VK_FORMAT_R32G32B32A32_SFLOAT;
		cubemapImageInfo.ViewType = VK_IMAGE_VIEW_TYPE_CUBE;
		cubemapImageInfo.ViewAspect = VK_IMAGE_ASPECT_COLOR_BIT;
		cubemapImageInfo.UsageFlags = VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_STORAGE_BIT;
		cubemapImageInfo.Layout = VK_IMAGE_LAYOUT_GENERAL;
		cubemapImageInfo.CreateFlags = VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;

		vk::Texture cubemap;
		cubemap.Setup(*VulkanApp, resolution, resolution, cubemapImageInfo, params, 1, 6);

		cubemap.SetLayout(VulkanApp->ComputeQueue, VulkanApp->CommandPoolCQ,
						  vk::layout::SetCubeImageLayoutFromComputeWriteToGraphicsShader);

		vk::TextureDescriptor mapDescriptor;
		mapDescriptor.LinkTexture(hdrTexture, 0, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE);
		mapDescriptor.LinkTexture(cubemap, 1, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE);
		mapDescriptor.Create(*VulkanApp, DescriptorPoolImageStorage);

		vk::ComputeShader cs;
		cs.Setup(*VulkanApp, FromHdrToCubemapShader);

		const int workGroups = 16;
		vk::RunComputeShader(*VulkanApp, cs, mapDescriptor.GetDescriptorInfo(),
							 resolution / workGroups, resolution / workGroups, 6);

		cs.Cleanup();
		hdrTexture.Cleanup();
		mapDescriptor.Destroy();

		size_t newId = AM->IncrementImageCounter();
		TM.AddTexture(newId, cubemap);

		return newId;
	}

	size_t RenderManager::GenerateIrradianceMap(const asset::AssetId id)
	{
		vk::TextureParams params;
		params.MagFilter = VK_FILTER_LINEAR;
		params.MinFilter = VK_FILTER_LINEAR;
		params.AddressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
		params.AddressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;

		vk::TextureImageInfo hdrImageInfo;
		hdrImageInfo.Type = VK_IMAGE_TYPE_2D;
		hdrImageInfo.Format = VK_FORMAT_R32G32B32A32_SFLOAT;
		hdrImageInfo.ViewType = VK_IMAGE_VIEW_TYPE_CUBE;
		hdrImageInfo.ViewAspect = VK_IMAGE_ASPECT_COLOR_BIT;
		hdrImageInfo.UsageFlags = VK_IMAGE_USAGE_TRANSFER_DST_BIT 
								  | VK_IMAGE_USAGE_SAMPLED_BIT 
							      | VK_IMAGE_USAGE_STORAGE_BIT;
		hdrImageInfo.Layout = VK_IMAGE_LAYOUT_GENERAL;
		hdrImageInfo.CreateFlags = VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;

		auto hdrTexture = TM.GetOrCreate({ id, params }, vk::DescriptorImageType::Cubemap);

		vk::TextureImageInfo mapImageInfo;
		mapImageInfo.Type = VK_IMAGE_TYPE_2D;
		mapImageInfo.Format = VK_FORMAT_R32G32B32A32_SFLOAT;
		mapImageInfo.ViewType = VK_IMAGE_VIEW_TYPE_CUBE;
		mapImageInfo.ViewAspect = VK_IMAGE_ASPECT_COLOR_BIT;
		mapImageInfo.UsageFlags = VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_STORAGE_BIT;
		mapImageInfo.Layout = VK_IMAGE_LAYOUT_GENERAL;
		mapImageInfo.CreateFlags = VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;

		vk::Texture map;
		map.Setup(*VulkanApp, 64, 64, mapImageInfo, params, 1, 6);

		map.SetLayout(VulkanApp->ComputeQueue, VulkanApp->CommandPoolCQ,
					  vk::layout::SetCubeImageLayoutFromComputeWriteToGraphicsShader);


		vk::TextureDescriptor mapDescriptor;
		mapDescriptor.LinkTexture(hdrTexture, 0, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE);
		mapDescriptor.LinkTexture(map, 1, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE);
		mapDescriptor.Create(*VulkanApp, DescriptorPoolImageStorage);

		vk::ComputeShader cs;
		cs.Setup(*VulkanApp, IrradianceMapComputeShader);

		const int workGroups = 16;
		vk::RunComputeShader(*VulkanApp, cs, mapDescriptor.GetDescriptorInfo(),
							 64 / workGroups, 64 / workGroups, 6);

		cs.Cleanup();
		mapDescriptor.Destroy();

		size_t newId = AM->IncrementImageCounter();
		TM.AddTexture(newId, map);

		return newId;
	}
}