#include "buffer.h"

#include "helpers.h"

namespace vk
{
	void Buffer::Setup(vk::VulkanApp& app, const VkBufferUsageFlags usageFlags, const size_t stride, const size_t elementsCount)
	{
		VulkanApp = &app;

		ElementsCount = elementsCount;
		Stride = stride;

		VkBufferCreateInfo bufferCreateInfo{};
		bufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		bufferCreateInfo.size = Stride * ElementsCount;
		bufferCreateInfo.usage = usageFlags;
		bufferCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

		auto res = vkCreateBuffer(VulkanApp->Device, &bufferCreateInfo, nullptr, &BufferH);
		ASSERT(res == VK_SUCCESS, "Buffer creation error!");

		VkMemoryRequirements memRequirements;
		vkGetBufferMemoryRequirements(VulkanApp->Device, BufferH, &memRequirements);

		VkMemoryAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		allocInfo.allocationSize = memRequirements.size;
		allocInfo.memoryTypeIndex = FindMemoryType(*VulkanApp, memRequirements.memoryTypeBits, 
												   VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT
												   | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

		res = vkAllocateMemory(VulkanApp->Device, &allocInfo, nullptr, &BufferMemory);
		ASSERT(res == VK_SUCCESS, "Couldn't allocate buffer memory!");

		vkBindBufferMemory(VulkanApp->Device, BufferH, BufferMemory, 0);
	}
}