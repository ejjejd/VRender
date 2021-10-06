#pragma once
#include "vrender.h"

#include "image.h"
#include "descriptor.h"
#include "pool.h"

namespace vk
{	
	struct TextureParams
	{
		VkFilter MagFilter;
		VkFilter MinFilter;
		VkSamplerAddressMode AddressModeU;
		VkSamplerAddressMode AddressModeV;
	};

	struct TextureImageInfo
	{
		VkImageType Type;
		VkImageViewType ViewType;
		VkFormat Format;
		VkImageAspectFlags ViewAspect;
		VkImageUsageFlags UsageFlags;
		VkImageLayout Layout;
		VkFlags CreateFlags = 0;
		ImageChannels Channels;
	};

	class Texture
	{
	private:
		vk::Image Image;
		VkSampler Sampler;

		VkDescriptorImageInfo Info;
		TextureImageInfo ImageInfo;

		vk::VulkanApp* App;
	public:
		bool Setup(vk::VulkanApp& app, const uint16_t width, const uint16_t height, 
				   const TextureImageInfo& imageInfo, const TextureParams& params,
				   const uint16_t depth = 1, const uint16_t layersCount = 1, const uint8_t mipLevels = 1);

		inline void Cleanup() const
		{
			vkDestroySampler(App->Device, Sampler, nullptr);

			Image.Cleanup();
		}

		void Update(void* data, const size_t pixelStride);

		inline void SetLayout(const VkQueue queue, const VkCommandPool commandPool,
						      void(*layoutFunc)(const vk::VulkanApp&, const VkQueue, const VkCommandPool, const VkImage, const uint8_t))
		{
			if (layoutFunc)
				layoutFunc(*App, queue, commandPool, Image.GetHandler(), Image.GetMipLevels());
		}

		inline vk::Image GetImage() const
		{
			return Image;
		}

		inline VkDescriptorImageInfo GetInfo() const
		{
			return Info;
		}
	};

	class TextureDescriptor
	{
	private:
		Descriptor DescriptorInfo;

		struct
		{
			std::vector<VkDescriptorSetLayoutBinding> LayoutBindInfos;
			std::vector<VkDescriptorImageInfo> ImageInfos;
		} ImageInfos;

		VulkanApp* App;
	public:
		void Create(VulkanApp& app, DescriptorPoolManager& pm, const VkDescriptorType type);

		inline void Destroy() const
		{
			CleanupDescriptor(*App, DescriptorInfo);
		}

		inline void LinkTexture(const vk::Texture& texture, const uint8_t bindId)
		{
			VkDescriptorSetLayoutBinding layoutBinding{};
			layoutBinding.binding = bindId;
			layoutBinding.descriptorCount = 1;
			layoutBinding.pImmutableSamplers = nullptr;
			layoutBinding.stageFlags = VK_SHADER_STAGE_ALL_GRAPHICS | VK_SHADER_STAGE_COMPUTE_BIT;

			ImageInfos.LayoutBindInfos.push_back({ layoutBinding });

			ImageInfos.ImageInfos.push_back(texture.GetInfo());
		}

		inline Descriptor GetDescriptorInfo() const
		{
			return DescriptorInfo;
		}
	};
}