#pragma once
#include "vrender.h"
#include "buffer.h"

enum class IC
{
	R,
	G,
	B,
	A,
	One,
	Zero
};

struct ImageChannels
{
	IC R = IC::R;
	IC G = IC::G;
	IC B = IC::B;
	IC A = IC::A;
};

//TODO replace texture manual init with one channel

namespace vk
{
	inline VkComponentMapping FromImageChannelsToSwizzleMap(const ImageChannels channels)
	{
		auto retSwizzle = 
			[](const IC c) -> auto
			{
				switch (c)
				{
				case IC::R: return VK_COMPONENT_SWIZZLE_R;
				case IC::G: return VK_COMPONENT_SWIZZLE_G;
				case IC::B: return VK_COMPONENT_SWIZZLE_B;
				case IC::A: return VK_COMPONENT_SWIZZLE_A;
				case IC::One: return VK_COMPONENT_SWIZZLE_ONE;
				}

				return VK_COMPONENT_SWIZZLE_ZERO;
			};

		VkComponentMapping map;
		map.r = retSwizzle(channels.R);
		map.g = retSwizzle(channels.G);
		map.b = retSwizzle(channels.B);
		map.a = retSwizzle(channels.A);

		return map;
	}

	class Image
	{
	private:
		VkImage Image;
		VkImageView ImageView;
		VkDeviceMemory Memory;

		uint16_t Width;
		uint16_t Height;

		uint8_t MipLevels;

		VulkanApp* App;
	public:
		bool Image::Setup(VulkanApp& app, const VkImageType type, const VkImageViewType viewType, 
						  const VkFormat format, const VkImageUsageFlags usage, const VkImageAspectFlags& viewAspect,
						  const uint16_t width, const uint16_t height, const uint16_t depth, const uint16_t layersCount,
						  const VkFlags flags, const ImageChannels channels, const uint8_t mipLevels);

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

		inline uint8_t GetMipLevels() const
		{
			return MipLevels;
		}
	};
}