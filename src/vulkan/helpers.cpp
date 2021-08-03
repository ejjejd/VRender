#include "helpers.h"

#include <fstream>

namespace vk
{
	std::optional<std::vector<char>> ReadShader(const std::string& filename)
	{
		std::ifstream file(filename, std::ios::ate | std::ios::binary);

		if (!file.is_open())
			return std::nullopt;

		size_t fileSize = (size_t)file.tellg();
		std::vector<char> bytecode(fileSize);

		file.seekg(0);
		file.read(bytecode.data(), fileSize);

		file.close();

		return bytecode;
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
}