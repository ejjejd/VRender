#include "texture.h"

#include "vulkan/buffer.h"
#include "vulkan/helpers.h"

namespace graphics
{
	void Texture::Update(void* data)
	{
		vk::Buffer buffer;
		buffer.Setup(*App, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, 4, Image.GetWidth() * Image.GetHeight());

		buffer.Update(data, Image.GetWidth() * Image.GetHeight());

		vk::TransitionImageLayout(*App, Image.GetHandler(), VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
		vk::CopyBufferToImage(*App, buffer.GetHandler(), Image.GetHandler(), Image.GetWidth(), Image.GetHeight());
		vk::TransitionImageLayout(*App, Image.GetHandler(), VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

		buffer.Cleanup();
	}
}