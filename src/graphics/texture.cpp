#include "texture.h"

#include "vulkan/buffer.h"
#include "vulkan/helpers.h"

namespace graphics
{
	bool Texture::Setup(vk::VulkanApp& app, const uint16_t width, const uint16_t height, const TextureParams& params)
	{
		App = &app;

		if (!Image.Setup(app, VK_IMAGE_TYPE_2D, VK_IMAGE_VIEW_TYPE_2D, VK_FORMAT_R8G8B8A8_SRGB,
			VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, width, height))
		{
			return false;
		}

		VkSamplerCreateInfo samplerCreateInfo{};
		samplerCreateInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
		samplerCreateInfo.magFilter = params.MagFilter;
		samplerCreateInfo.minFilter = params.MinFilter;
		samplerCreateInfo.addressModeU = params.AddressModeU;
		samplerCreateInfo.addressModeV = params.AddressModeV;
		samplerCreateInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		samplerCreateInfo.anisotropyEnable = VK_TRUE;
		samplerCreateInfo.maxAnisotropy = app.DeviceProperties.limits.maxSamplerAnisotropy;
		samplerCreateInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
		samplerCreateInfo.unnormalizedCoordinates = VK_FALSE;
		samplerCreateInfo.compareEnable = VK_FALSE;
		samplerCreateInfo.compareOp = VK_COMPARE_OP_ALWAYS;
		samplerCreateInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
		samplerCreateInfo.mipLodBias = 0.0f;
		samplerCreateInfo.minLod = 0.0f;
		samplerCreateInfo.maxLod = 0.0f;

		if (vkCreateSampler(app.Device, &samplerCreateInfo, nullptr, &Sampler) != VK_SUCCESS)
			return false;

		return true;
	}

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
namespace vk
{
	void TextureDescriptor::Create(vk::VulkanApp& app, const VkDescriptorPool& descriptorPool)
	{
		App = &app;

		VkDescriptorSetLayoutCreateInfo layoutCreateInfo{};
		layoutCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		layoutCreateInfo.bindingCount = ImageInfos.LayoutBindInfos.size();
		layoutCreateInfo.pBindings = ImageInfos.LayoutBindInfos.data();

		if (vkCreateDescriptorSetLayout(app.Device, &layoutCreateInfo, nullptr, &DescriptorInfo.DescriptorSetLayout) != VK_SUCCESS)
		{
			TERMINATE_LOG("Couldn't create descriptor set layout!");
			return;
		}


		std::vector<VkDescriptorSetLayout> descriptorLayoutsCopies(1, DescriptorInfo.DescriptorSetLayout);

		VkDescriptorSetAllocateInfo descriptorAllocInfo{};
		descriptorAllocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		descriptorAllocInfo.descriptorPool = descriptorPool;
		descriptorAllocInfo.descriptorSetCount = descriptorLayoutsCopies.size();
		descriptorAllocInfo.pSetLayouts = descriptorLayoutsCopies.data();


		DescriptorInfo.DescriptorSets.resize(descriptorLayoutsCopies.size());

		if (vkAllocateDescriptorSets(app.Device, &descriptorAllocInfo, &DescriptorInfo.DescriptorSets[0]) != VK_SUCCESS)
		{
			TERMINATE_LOG("Couldn't create descriptor set layout!");
			return;
		}

		for (size_t j = 0; j < ImageInfos.ImageInfos.size(); ++j)
		{
			VkWriteDescriptorSet descriptorWrite{};
			descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			descriptorWrite.dstSet = DescriptorInfo.DescriptorSets[0];
			descriptorWrite.dstBinding = ImageInfos.LayoutBindInfos[j].binding;
			descriptorWrite.dstArrayElement = 0;
			descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
			descriptorWrite.descriptorCount = 1;
			descriptorWrite.pImageInfo = &ImageInfos.ImageInfos[j];

			vkUpdateDescriptorSets(app.Device, 1, &descriptorWrite, 0, nullptr);
		}
	}
}