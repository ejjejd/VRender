#include "render_manager.h"

#include <set>
#include <fstream>

namespace manager
{
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

		vkDestroyRenderPass(app.Device, RenderPass, nullptr);

		vkDestroySemaphore(app.Device, ImageAvailableSemaphore, nullptr);
		vkDestroySemaphore(app.Device, RenderFinishedSemaphore, nullptr);
	}

	bool RenderManager::Update(vk::VulkanApp& app, const std::vector<VkCommandBuffer>& commandBuffers)
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
		submitInfo.pCommandBuffers = &commandBuffers[imageId];

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