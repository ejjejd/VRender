#pragma once
#include "vrender.h"
#include "vulkan/image.h"
#include "vulkan/descriptor.h"

namespace graphics
{
	struct TextureParams
	{
		VkFilter MagFilter;
		VkFilter MinFilter;
		VkSamplerAddressMode AddressModeU;
		VkSamplerAddressMode AddressModeV;
	};

	class Texture
	{
	private:
		vk::Image Image;
		VkSampler Sampler;

		vk::VulkanApp* App;
	public:
		bool Setup(vk::VulkanApp& app, const uint16_t width, const uint16_t height, const TextureParams& params);

		inline void Cleanup()
		{
			vkDestroySampler(App->Device, Sampler, nullptr);

			Image.Cleanup();
		}

		void Update(void* data);

		inline vk::Image GetImage() const
		{
			return Image;
		}

		inline VkDescriptorImageInfo GetInfo() const
		{
			VkDescriptorImageInfo info{};
			info.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
			info.imageView = Image.GetViewHandler();
			info.sampler = Sampler;

			return info;
		}
	};
}

namespace vk
{
	class TextureDescriptor
	{
	private:
		vk::Descriptor DescriptorInfo;

		struct
		{
			std::vector<VkDescriptorSetLayoutBinding> LayoutBindInfos;
			std::vector<VkDescriptorImageInfo> ImageInfos;
		} ImageInfos;

		vk::VulkanApp* App;
	public:
		void Create(vk::VulkanApp& app, const VkDescriptorPool& descriptorPool);

		inline void Destroy() const
		{
			CleanupDescriptor(*App, DescriptorInfo);
		}

		inline void LinkTexture(const graphics::Texture& texture, const uint8_t bindId)
		{
			VkDescriptorSetLayoutBinding layoutBinding{};
			layoutBinding.binding = bindId;
			layoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
			layoutBinding.descriptorCount = 1;
			layoutBinding.pImmutableSamplers = nullptr;
			layoutBinding.stageFlags = VK_SHADER_STAGE_ALL_GRAPHICS;

			ImageInfos.LayoutBindInfos.push_back({ layoutBinding });

			ImageInfos.ImageInfos.push_back(texture.GetInfo());
		}

		inline vk::Descriptor GetDescriptorInfo() const
		{
			return DescriptorInfo;
		}
	};
}