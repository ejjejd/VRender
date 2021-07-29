#pragma once
#include "vrender.h"

#include "buffer.h"
#include "descriptor.h"

namespace graphics
{
	class UniformBuffer
	{
	private:
		std::vector<Buffer> Buffers;

		vk::VulkanApp* VulkanApp;
	public:
		inline void Setup(vk::VulkanApp& app, const uint8_t buffersCount, const size_t stride, const size_t elementsCount)
		{
			VulkanApp = &app;

			Buffers.resize(buffersCount);
			for (auto& b : Buffers)
				b.Setup(app, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, stride, elementsCount);
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
		}

		Descriptor CreateDescriptor(const VkDescriptorPool& descriptorPool, const uint8_t binding);
	};
}