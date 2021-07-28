#pragma once
#include "vrender.h"
#include "vulkan/vulkan_app.h"

namespace graphics
{
	class API Buffer
	{
	private:
		VkBuffer BufferH;
		VkDeviceMemory BufferMemory;

		vk::VulkanApp* VulkanApp;

		size_t ElementsCount;
		size_t Stride;
	public:
		void Setup(vk::VulkanApp& app, const VkBufferUsageFlags usageFlags, const size_t stride, const size_t elementsCount);

		inline void Update(void* data, const size_t elementsCount)
		{
			if (elementsCount > ElementsCount)
			{
				LOG("Wrong data passed to update buffer!");
				return;
			}

			void* mapPtr;
			vkMapMemory(VulkanApp->Device, BufferMemory, 0, Stride * elementsCount, 0, &mapPtr);
			memcpy(mapPtr, data, Stride * elementsCount);
			vkUnmapMemory(VulkanApp->Device, BufferMemory);
		}

		inline void Cleanup()
		{
			vkDestroyBuffer(VulkanApp->Device, BufferH, nullptr);
			vkFreeMemory(VulkanApp->Device, BufferMemory, nullptr);
		}

		inline VkBuffer GetHandler() const
		{
			return BufferH;
		}

		inline size_t GetElementsCount() const
		{
			return ElementsCount;
		}

		inline size_t GetStride() const
		{
			return Stride;
		}
	};
}