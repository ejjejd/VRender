#pragma once
#include "vrender.h"

#include <vector>
#include <unordered_map>

#include "vulkan_app.h"

namespace vk
{
	constexpr uint8_t SetsLimitPerDescriptorPool = 64;

	struct DescriptorPoolUnit
	{
		VkDescriptorType DescriptorType;
		size_t DescriptorsCount;
	};

	class DescriptorPoolManager
	{
	private:
		std::vector<VkDescriptorPool> DescriptorPools;
		size_t LastlyUsedPoolId = 0;

		std::unordered_map<size_t, DescriptorPoolUnit> UnitsMap;

		VulkanApp* App;

		[[nodiscard]]
		bool PushDescriptorPool();
	public:
		inline void Setup(VulkanApp& app)
		{
			App = &app;
		}

		inline void Cleanup()
		{
			ASSERT(App, "Descriptor manager wasn't setted up before use!");

			for (const auto& p : DescriptorPools)
				vkDestroyDescriptorPool(App->Device, p, nullptr);
		}

		inline void Recreate()
		{
			Cleanup();

			DescriptorPools.clear();
			LastlyUsedPoolId = 0;

			ASSERT(PushDescriptorPool(), "Couldn't create descriptor pool!");
		}

		std::vector<VkDescriptorSet> GetAllocatedSets(const VkDescriptorType type, 
													  const std::vector<VkDescriptorSetLayout>& layouts);

		inline void AddUnit(const VkDescriptorType type)
		{
			UnitsMap[type] = { type, SetsLimitPerDescriptorPool };
		}

		inline void ClearUnits()
		{
			UnitsMap.clear();
		}
	};
}