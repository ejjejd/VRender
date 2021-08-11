#pragma once
#include "vrender.h"
#include "vulkan/image.h"

namespace graphics
{
	class Texture
	{
	private:
		vk::Image Image;

		vk::VulkanApp* App;
	public:
		inline void Setup(vk::VulkanApp& app, const uint16_t width, const uint16_t height)
		{
			App = &app;

			Image.Setup(app, VK_IMAGE_TYPE_2D, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, width, height);
		}

		inline void Cleanup()
		{
			Image.Cleanup();
		}

		void Update(void* data);
	};
}