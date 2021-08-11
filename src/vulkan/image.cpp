#include "image.h"

#include "helpers.h"

namespace vk
{
	bool Image::Setup(VulkanApp& app, const VkImageType type, const VkFormat format, const VkImageUsageFlags usage, 
					  const uint16_t width, const uint16_t height)
	{
		App = &app;

		Width = width;
		Height = height;

		VkImageCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		createInfo.imageType = type;
		createInfo.extent.width = width;
		createInfo.extent.height = height;
		createInfo.extent.depth = 1;
		createInfo.mipLevels = 1;
		createInfo.arrayLayers = 1;
		createInfo.format = format;
		createInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
		createInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		createInfo.usage = usage;
		createInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		createInfo.samples = VK_SAMPLE_COUNT_1_BIT;
		createInfo.flags = 0;

		if (vkCreateImage(app.Device, &createInfo, nullptr, &Image) != VK_SUCCESS)
			return false;

		VkMemoryRequirements memRequirements;
		vkGetImageMemoryRequirements(app.Device, Image, &memRequirements);

		VkMemoryAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		allocInfo.allocationSize = memRequirements.size;
		allocInfo.memoryTypeIndex = FindMemoryType(app, memRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

		if (vkAllocateMemory(app.Device, &allocInfo, nullptr, &Memory) != VK_SUCCESS)
			return false;

		vkBindImageMemory(app.Device, Image, Memory, 0);
	}

	void Image::Cleanup()
	{
		vkDestroyImage(App->Device, Image, nullptr);
		vkFreeMemory(App->Device, Memory, nullptr);
	}
}