#pragma once
#include "vrender.h"

#include "buffer.h"
#include "descriptor.h"

namespace graphics
{
	//Dynamic use more buffer to properly update them in main loop
	enum class UboType
	{
		Static,
		Dynamic
	};

	class UniformBuffer
	{
	private:
		std::vector<Buffer> Buffers;

		UboType Type;

		vk::VulkanApp* VulkanApp;
	public:
		void Setup(vk::VulkanApp& app, const UboType type, const size_t stride, const size_t elementsCount);

		//Use this only for dynamic buffer
		inline void Update(const size_t imageId, void* data, const size_t elementsCount)
		{
			if (imageId >= Buffers.size()
			    || Type == UboType::Static)
			{
				LOG("Invalid buffer id passed or you use static type buffer: %d", imageId)
				return;
			}

			Buffers[imageId].Update(data, elementsCount);
		}
		
		inline void Update(void* data, const size_t elementsCount)
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