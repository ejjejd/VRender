#pragma once
#include "vrender.h"

#include "buffer.h"

namespace graphics
{
	class UniformBuffer;

	struct Descriptor
	{
		VkDescriptorSetLayout DescriptorSetLayout;
		std::vector<VkDescriptorSet> DescriptorSets;
	};

	Descriptor CreateDescriptor(vk::VulkanApp& app, const UniformBuffer& ubo, const VkDescriptorPool& descriptorPool, const uint8_t binding);

	inline void CleanupDescriptor(const vk::VulkanApp& app, const Descriptor& descriptor)
	{
		vkDestroyDescriptorSetLayout(app.Device, descriptor.DescriptorSetLayout, nullptr);
	}

	class UniformBuffer
	{
	private:
		std::vector<Buffer> Buffers;

		VkDescriptorPool DescriptorPool;

		vk::VulkanApp* VulkanApp;

		friend Descriptor CreateDescriptor(vk::VulkanApp& app, const UniformBuffer& ubo, const VkDescriptorPool& descriptorPool, const uint8_t binding);
	public:
		inline void Setup(vk::VulkanApp& app, const uint8_t buffersCount, const size_t stride, const size_t elementsCount)
		{
			VulkanApp = &app;

			Buffers.resize(buffersCount);
			for (auto& b : Buffers)
				b.Setup(app, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, stride, elementsCount);
			
			VkDescriptorPoolSize poolSize{};
			poolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			poolSize.descriptorCount = Buffers.size();

			VkDescriptorPoolCreateInfo poolCreateInfo{};
			poolCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
			poolCreateInfo.poolSizeCount = 1;
			poolCreateInfo.pPoolSizes = &poolSize;
			poolCreateInfo.maxSets = Buffers.size();

			if (vkCreateDescriptorPool(VulkanApp->Device, &poolCreateInfo, nullptr, &DescriptorPool) != VK_SUCCESS)
			{
				LOG("Couldn't create descriptor pool!");
				return;
			}
		}

		inline void UpdateBuffer(const size_t id, void* data, const size_t elementsCount)
		{
			if (id >= Buffers.size())
			{
				LOG("Invalid buffer id passed during uniform buffer update: %d", id)
				return;
			}

			Buffers[id].Update(data, elementsCount);
		}
		
		inline void UpdateBuffers(void* data, const size_t elementsCount)
		{
			for (auto& b : Buffers)
				b.Update(data, elementsCount);
		}

		inline void Cleanup()
		{
			for (auto& b : Buffers)
				b.Cleanup();
			vkDestroyDescriptorPool(VulkanApp->Device, DescriptorPool, nullptr);
		}
	};
}