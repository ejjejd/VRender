#pragma once
#include <optional>

#include "vrender.h"
#include "vulkan_app.h"


namespace vk
{
	std::optional<std::vector<char>> ReadShader(const std::string& filename);

	int32_t FindMemoryType(const vk::VulkanApp& app, uint32_t typeFilter, VkMemoryPropertyFlags properties);
}