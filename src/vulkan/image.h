#pragma once
#include "vrender.h"
#include "buffer.h"

namespace vk
{
	class Image
	{
	private:
		VkImage Image;
		VkImageView ImageView;
		VkDeviceMemory Memory;

		uint16_t Width;
		uint16_t Height;

		VulkanApp* App;
	public:
		bool Image::Setup(VulkanApp& app, const VkImageType type, const VkImageViewType viewType, 
						  const VkFormat format, const VkImageUsageFlags usage, const VkImageAspectFlags& viewAspect,
						  const uint16_t width, const uint16_t height);
		void Cleanup() const;

		inline VkImage GetHandler() const
		{
			return Image;
		}

		inline VkImageView GetViewHandler() const
		{
			return ImageView;
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