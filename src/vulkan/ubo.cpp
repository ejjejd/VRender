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

	void UboDescriptor::Create(VulkanApp& app, const VkDescriptorPool& descriptorPool)
	{
		App = &app;

		VkDescriptorSetLayoutCreateInfo layoutCreateInfo{};
		layoutCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		layoutCreateInfo.bindingCount = UboInfos.LayoutBindInfos.size();
		layoutCreateInfo.pBindings = UboInfos.LayoutBindInfos.data();

		if (vkCreateDescriptorSetLayout(app.Device, &layoutCreateInfo, nullptr, &DescriptorInfo.DescriptorSetLayout) != VK_SUCCESS)
		{
			TERMINATE_LOG("Couldn't create descriptor set layout!");
			return;
		}

		std::vector<VkDescriptorSetLayout> descriptorLayoutsCopies(app.SwapChainImages.size(), DescriptorInfo.DescriptorSetLayout);

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

		for (size_t i = 0; i < UboInfos.BufferInfos.size(); ++i)
		{
			for (size_t j = 0; j < UboInfos.BufferInfos[i].size(); ++j)
			{
				VkWriteDescriptorSet descriptorWrite{};
				descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
				descriptorWrite.dstSet = DescriptorInfo.DescriptorSets[j];
				descriptorWrite.dstBinding = UboInfos.LayoutBindInfos[i].binding;
				descriptorWrite.dstArrayElement = 0;
				descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
				descriptorWrite.descriptorCount = 1;
				descriptorWrite.pBufferInfo = UboInfos.BufferInfos[i].data();

				vkUpdateDescriptorSets(app.Device, 1, &descriptorWrite, 0, nullptr);
			}
		}
	}
}