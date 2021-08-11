#pragma once
#include "vrender.h"
#include "buffer.h"

namespace vk
{
	class Image
	{
	private:
		VkImage Image;
		VkDeviceMemory Memory;

		uint16_t Width;
		uint16_t Height;

		VulkanApp* App;
	public:
		bool Image::Setup(VulkanApp& app, const VkImageType type, const VkFormat format, const VkImageUsageFlags usage,
						  const uint16_t width, const uint16_t height);
		void Cleanup();

		inline VkImage GetHandler() const
		{
			return Image;
		}

		inline uint16_t GetWidth() const
		{
			return Width;
		}

		inline uint16_t GetHeight() const
		{
			return Height;
		}
	};
}