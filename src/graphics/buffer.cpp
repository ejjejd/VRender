#include "buffer.h"

namespace graphics
{
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

	void Buffer::Init(void* data, const size_t stride, const size_t elementsCount)
	{
		ElementsCount = elementsCount;
		Stride = stride;

		VkBufferCreateInfo bufferCreateInfo{};
		bufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		bufferCreateInfo.size = Stride * ElementsCount;
		bufferCreateInfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
		bufferCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

		if (vkCreateBuffer(VulkanApp.get().Device, &bufferCreateInfo, nullptr, &BufferH) != VK_SUCCESS)
		{
			LOG("Buffer creation error!");
			return;
		}

		VkMemoryRequirements memRequirements;
		vkGetBufferMemoryRequirements(VulkanApp.get().Device, BufferH, &memRequirements);

		VkMemoryAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		allocInfo.allocationSize = memRequirements.size;
		allocInfo.memoryTypeIndex = FindMemoryType(VulkanApp, memRequirements.memoryTypeBits, 
												   VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT
												   | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

		if (vkAllocateMemory(VulkanApp.get().Device, &allocInfo, nullptr, &BufferMemory) != VK_SUCCESS)
		{
			LOG("Couldn't allocate buffer memory!");
			return;
		}

		vkBindBufferMemory(VulkanApp.get().Device, BufferH, BufferMemory, 0);

		void* mapPtr;
		vkMapMemory(VulkanApp.get().Device, BufferMemory, 0, bufferCreateInfo.size, 0, &mapPtr);
		memcpy(mapPtr, data, bufferCreateInfo.size);
		vkUnmapMemory(VulkanApp.get().Device, BufferMemory);
	}
}