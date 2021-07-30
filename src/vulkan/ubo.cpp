#include "ubo.h"

namespace vk
{
	void UniformBuffer::Setup(vk::VulkanApp& app, const UboType type, const size_t stride, const size_t elementsCount)
	{
		VulkanApp = &app;
		Type = type;

		size_t vSize = 0;

		if (type == UboType::Dynamic)
			vSize = VulkanApp->SwapChainImageViews.size();
		else
			vSize = 1;

		Buffers.resize(vSize);
		for (auto& b : Buffers)
			b.Setup(app, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, stride, elementsCount);
	}

	Descriptor UniformBuffer::CreateDescriptor(const VkDescriptorPool& descriptorPool, const uint8_t binding)
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

		if (vkCreateDescriptorSetLayout(VulkanApp->Device, &layoutInfo, nullptr, &descriptor.DescriptorSetLayout) != VK_SUCCESS)
		{
			LOG("Couldn't create descriptor layout!");
			return descriptor;
		}

		std::vector<VkDescriptorSetLayout> layouts(Buffers.size(), descriptor.DescriptorSetLayout);

		VkDescriptorSetAllocateInfo descAllocInfo{};
		descAllocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		descAllocInfo.descriptorPool = descriptorPool;
		descAllocInfo.descriptorSetCount = Buffers.size();
		descAllocInfo.pSetLayouts = layouts.data();

		descriptor.DescriptorSets.resize(Buffers.size());
		if (vkAllocateDescriptorSets(VulkanApp->Device, &descAllocInfo, &descriptor.DescriptorSets[0]) != VK_SUCCESS)
		{
			LOG("Couldn't allocate descriptor sets!");
			return descriptor;
		}

		for (size_t i = 0; i < Buffers.size(); ++i)
		{
			VkDescriptorBufferInfo bufferInfo{};
			bufferInfo.buffer = Buffers[i].GetHandler();
			bufferInfo.offset = 0;
			bufferInfo.range = Buffers[i].GetStride();

			VkWriteDescriptorSet descriptorWrite{};
			descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			descriptorWrite.dstSet = descriptor.DescriptorSets[i];
			descriptorWrite.dstBinding = binding;
			descriptorWrite.dstArrayElement = 0;
			descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			descriptorWrite.descriptorCount = 1;
			descriptorWrite.pBufferInfo = &bufferInfo;

			vkUpdateDescriptorSets(VulkanApp->Device, 1, &descriptorWrite, 0, nullptr);
		}

		return descriptor;
	}
}