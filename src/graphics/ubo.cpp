#include "ubo.h"

namespace graphics
{
	Descriptor CreateDescriptor(vk::VulkanApp& app, const UniformBuffer& ubo, const VkDescriptorPool& descriptorPool, const uint8_t binding)
	{
		VkDescriptorSetLayoutBinding layoutBinding{};
		layoutBinding.binding = binding;
		layoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		layoutBinding.descriptorCount = 1;
		layoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

		VkDescriptorSetLayoutCreateInfo layoutInfo{};
		layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		layoutInfo.bindingCount = 1;
		layoutInfo.pBindings = &layoutBinding;

		Descriptor descriptor;

		if (vkCreateDescriptorSetLayout(app.Device, &layoutInfo, nullptr, &descriptor.DescriptorSetLayout) != VK_SUCCESS)
		{
			LOG("Couldn't create descriptor layout!");
			return descriptor;
		}

		std::vector<VkDescriptorSetLayout> layouts(ubo.Buffers.size(), descriptor.DescriptorSetLayout);

		VkDescriptorSetAllocateInfo descAllocInfo{};
		descAllocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		descAllocInfo.descriptorPool = descriptorPool;
		descAllocInfo.descriptorSetCount = ubo.Buffers.size();
		descAllocInfo.pSetLayouts = layouts.data();

		descriptor.DescriptorSets.resize(ubo.Buffers.size());
		if (vkAllocateDescriptorSets(app.Device, &descAllocInfo, &descriptor.DescriptorSets[0]) != VK_SUCCESS)
		{
			LOG("Couldn't allocate descriptor sets!");
			return descriptor;
		}

		for (size_t i = 0; i < ubo.Buffers.size(); ++i)
		{
			VkDescriptorBufferInfo bufferInfo{};
			bufferInfo.buffer = ubo.Buffers[i].GetHandler();
			bufferInfo.offset = 0;
			bufferInfo.range = ubo.Buffers[i].GetStride();

			VkWriteDescriptorSet descriptorWrite{};
			descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			descriptorWrite.dstSet = descriptor.DescriptorSets[i];
			descriptorWrite.dstBinding = 0;
			descriptorWrite.dstArrayElement = 0;
			descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			descriptorWrite.descriptorCount = 1;
			descriptorWrite.pBufferInfo = &bufferInfo;

			vkUpdateDescriptorSets(app.Device, 1, &descriptorWrite, 0, nullptr);
		}

		return descriptor;
	}
}