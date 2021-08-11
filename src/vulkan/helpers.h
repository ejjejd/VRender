#pragma once
#include <optional>

#include "vrender.h"
#include "vulkan_app.h"


namespace vk
{
	std::optional<std::vector<char>> ReadShader(const std::string& filename);

	int32_t FindMemoryType(const vk::VulkanApp& app, uint32_t typeFilter, VkMemoryPropertyFlags properties);

	VkCommandBuffer BeginCommands(const VulkanApp& app, const VkCommandPool commandPool);
	void EndCommands(const VulkanApp& app, const VkCommandPool commandPool, const VkCommandBuffer commandBuffer, const VkQueue queue);

	void TransitionImageLayout(const VulkanApp& app, const VkImage image, const VkFormat format, const VkImageLayout oldLayout, const VkImageLayout newLayout);

	void CopyBufferToImage(const VulkanApp& app, const VkBuffer buffer, const VkImage image, const uint16_t width, const uint16_t height);
}