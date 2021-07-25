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

		std::reference_wrapper<vk::VulkanApp> VulkanApp;

		size_t ElementsCount;
		size_t Stride;
	public:
		inline Buffer(vk::VulkanApp& app)
			: VulkanApp(app) {}

		inline ~Buffer()
		{
			vkDestroyBuffer(VulkanApp.get().Device, BufferH, nullptr);
			vkFreeMemory(VulkanApp.get().Device, BufferMemory, nullptr);
		}

		void Init(void* data, const size_t stride, const size_t elementsCount);

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