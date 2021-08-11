#include "render_manager.h"

#include <set>
#include <fstream>

#include "vendors/stb/stb_image.h"

namespace manager
{
	struct CameraUboInfo
	{
		glm::mat4 ToCamera;
		glm::mat4 ToClip;
		glm::vec4 CameraPosition;
	};

	bool RenderManager::CreateRenderPass()
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


		VkAttachmentDescription attachments[2] = { colorAttachment, depthAttachment };

		VkRenderPassCreateInfo renderPassInfo{};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
		renderPassInfo.attachmentCount = 2;
		renderPassInfo.pAttachments = &attachments[0];
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

		if (vkCreateRenderPass(VulkanApp->Device, &renderPassInfo, nullptr, &RenderPass) != VK_SUCCESS)
			return false;

		Framebuffers.resize(VulkanApp->SwapChainImageViews.size());

		for (size_t i = 0; i < VulkanApp->SwapChainImageViews.size(); ++i)
		{
			VkImageView attachments[2];
			attachments[0] = VulkanApp->SwapChainImageViews[i];
			attachments[1] = VulkanApp->DepthImageView;

			VkFramebufferCreateInfo fboInfo{};
			fboInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
			fboInfo.renderPass = RenderPass;
			fboInfo.attachmentCount = 2;
			fboInfo.pAttachments = &attachments[0];
			fboInfo.width = VulkanApp->SwapChainExtent.width;
			fboInfo.height = VulkanApp->SwapChainExtent.height;
			fboInfo.layers = 1;

			if (vkCreateFramebuffer(VulkanApp->Device, &fboInfo, nullptr, &Framebuffers[i]) != VK_SUCCESS)
				return false;
		}

		return true;
	}

	void RenderManager::UpdateUBO(const uint8_t imageId)
	{
		CameraUboInfo ubo;
		ubo.ToCamera = ActiveCamera.GetViewMatrix();
		ubo.ToClip = ActiveCamera.GetProjection();
		ubo.CameraPosition = { ActiveCamera.Position, 1.0f };

		GlobalUBO.Update(&ubo, 1);
	}

	bool RenderManager::Setup(vk::VulkanApp& app)
	{
		VulkanApp = &app;

		if (!CreateRenderPass())
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

		int texWidth, texHeight, texChannels;
		stbi_uc* pixels = stbi_load("res/textures/test.jpg", &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);

		Texture.Setup(app, texWidth, texHeight);
		Texture.Update(pixels);

		return true;
	}

	void RenderManager::Cleanup()
	{
		GlobalUBO.Cleanup();

		vkFreeCommandBuffers(VulkanApp->Device, VulkanApp->CommandPoolGQ, CommandBuffers.size(), &CommandBuffers[0]);

		for (size_t i = 0; i < Framebuffers.size(); i++)
			vkDestroyFramebuffer(VulkanApp->Device, Framebuffers[i], nullptr);

		vkDestroyRenderPass(VulkanApp->Device, RenderPass, nullptr);

		vkDestroySemaphore(VulkanApp->Device, ImageAvailableSemaphore, nullptr);
		vkDestroySemaphore(VulkanApp->Device, RenderFinishedSemaphore, nullptr);
	}

	bool RenderManager::Update(const std::vector<render::Renderable>& renderables)
	{
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
			renderPassInfo.renderArea.extent = VulkanApp->SwapChainExtent;

			VkClearValue clearColor = { 0.0f, 0.0f, 0.0f, 1.0f };

			VkClearValue clearDepth;
			clearDepth.depthStencil.depth = 1.0f;

			VkClearValue clearValues[2] = { clearColor, clearDepth };

			renderPassInfo.clearValueCount = 2;
			renderPassInfo.pClearValues = &clearValues[0];

			vkCmdBeginRenderPass(CommandBuffers[i], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

			for (const auto& r : renderables)
			{
				vkCmdBindPipeline(CommandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, r.GraphicsPipeline);

				std::vector<VkDeviceSize> offsets(r.Buffers.size(), 0);

				vkCmdBindVertexBuffers(CommandBuffers[i], 0, r.Buffers.size(), r.Buffers.data(), offsets.data());


				std::vector<VkDescriptorSet> descriptors;

				for (auto d : r.Descriptors)
				{
					auto descriptorSets = d.GetDescriptorInfo().DescriptorSets;

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
				return false;
		}

		uint32_t imageId = 0;
		VkResult acqResult = vkAcquireNextImageKHR(VulkanApp->Device, VulkanApp->SwapChain, UINT64_MAX, ImageAvailableSemaphore, VK_NULL_HANDLE, &imageId);


		UpdateUBO(imageId);


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
			return false;

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

		return true;
	}
}