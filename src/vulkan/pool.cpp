#include "pool.h"

namespace vk
{
	bool DescriptorPoolManager::PushDescriptorPool()
	{
		ASSERT(App, "Descriptor manager wasn't setted up before use!");

		std::vector<VkDescriptorPoolSize> poolSizes;

		for (const auto& [t, u] : UnitsMap)
		{
			VkDescriptorPoolSize s{};
			s.type = u.DescriptorType;
			s.descriptorCount = u.DescriptorsCount;

			poolSizes.push_back(s);
		}

		VkDescriptorPoolCreateInfo poolCI{};
		poolCI.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		poolCI.poolSizeCount = poolSizes.size();
		poolCI.pPoolSizes = poolSizes.data();
		poolCI.maxSets = SetsLimitPerDescriptorPool;

		VkDescriptorPool pool;
		auto res = vkCreateDescriptorPool(App->Device, &poolCI, nullptr, &pool);

		if (res != VK_SUCCESS)
			return false;

		DescriptorPools.push_back(pool);
		return true;
	}

	std::vector<VkDescriptorSet> DescriptorPoolManager::GetAllocatedSets(const VkDescriptorType type,
																		 const std::vector<VkDescriptorSetLayout>& layouts)
	{
		ASSERT(UnitsMap.find(type) != UnitsMap.end(), "Descriptor pool manager couldn't allocated this type of set");

		VkDescriptorSetAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		allocInfo.descriptorPool = DescriptorPools[LastlyUsedPoolId];
		allocInfo.descriptorSetCount = layouts.size();
		allocInfo.pSetLayouts = layouts.data();

		std::vector<VkDescriptorSet> descriptors(layouts.size());

		auto res = vkAllocateDescriptorSets(App->Device, &allocInfo, &descriptors[0]);
		if (res != VK_SUCCESS)
		{
			ASSERT(PushDescriptorPool(), "Couldn't create descriptor pool!");
			LastlyUsedPoolId++;

			auto res = vkAllocateDescriptorSets(App->Device, &allocInfo, &descriptors[0]);
			ASSERT(res == VK_SUCCESS, "Error in descriptor set allocation!");
		}

		return descriptors;
	}
}