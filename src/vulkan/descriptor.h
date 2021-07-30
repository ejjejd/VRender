#pragma once
#include "vrender.h"
#include "vulkan/vulkan_app.h"

namespace vk
{
	struct Descriptor
	{
		VkDescriptorSetLayout DescriptorSetLayout;
		std::vector<VkDescriptorSet> DescriptorSets;
	};

	inline void CleanupDescriptor(const vk::VulkanApp& app, const Descriptor& descriptor)
	{
		vkDestroyDescriptorSetLayout(app.Device, descriptor.DescriptorSetLayout, nullptr);
	}
}