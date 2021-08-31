#include "procedulars_manager.h"

namespace manager
{
	size_t ProceduralsManager::GenerateIrradianceMap(const asset::AssetId id)
	{
		vk::TextureParams params;
		params.MagFilter = VK_FILTER_LINEAR;
		params.MinFilter = VK_FILTER_LINEAR;
		params.AddressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
		params.AddressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;

		vk::TextureImageInfo hdrImageInfo;
		hdrImageInfo.Type = VK_IMAGE_TYPE_2D;
		hdrImageInfo.Format = VK_FORMAT_R32G32B32A32_SFLOAT;
		hdrImageInfo.ViewType = VK_IMAGE_VIEW_TYPE_2D;
		hdrImageInfo.ViewAspect = VK_IMAGE_ASPECT_COLOR_BIT;
		hdrImageInfo.UsageFlags = VK_IMAGE_USAGE_TRANSFER_DST_BIT
			| VK_IMAGE_USAGE_SAMPLED_BIT
			| VK_IMAGE_USAGE_STORAGE_BIT;
		hdrImageInfo.Layout = VK_IMAGE_LAYOUT_GENERAL;

		auto hdrData = AM->GetImageInfo(id);
		vk::Texture hdrTexture;
		hdrTexture.Setup(*VulkanApp, hdrData.Width, hdrData.Height, hdrImageInfo, params);
		hdrTexture.Update(hdrData.PixelsData.data(), 4 * sizeof(float));

		hdrTexture.SetLayout(VulkanApp->ComputeQueue, VulkanApp->CommandPoolCQ,
			vk::layout::SetImageLayoutFromTransferToComputeRead);


		vk::TextureImageInfo imageInfo;
		imageInfo.Type = VK_IMAGE_TYPE_2D;
		imageInfo.Format = VK_FORMAT_R32G32B32A32_SFLOAT;
		imageInfo.ViewType = VK_IMAGE_VIEW_TYPE_CUBE;
		imageInfo.ViewAspect = VK_IMAGE_ASPECT_COLOR_BIT;
		imageInfo.UsageFlags = VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_STORAGE_BIT;
		imageInfo.Layout = VK_IMAGE_LAYOUT_GENERAL;
		imageInfo.CreateFlags = VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;

		vk::Texture map;
		map.Setup(*VulkanApp, 64, 64, imageInfo, params, 1, 6);

		map.SetLayout(VulkanApp->ComputeQueue, VulkanApp->CommandPoolCQ,
			vk::layout::SetCubeImageLayoutFromComputeWriteToGraphicsShader);


		vk::TextureDescriptor mapDescriptor;
		mapDescriptor.LinkTexture(hdrTexture, 0, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE);
		mapDescriptor.LinkTexture(map, 1, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE);
		mapDescriptor.Create(*VulkanApp, DescriptorPoolImageStorage);

		vk::ComputeShader cs;
		cs.Setup(*VulkanApp, "res/shaders/compute/generate_im.spv");

		std::vector<VkDescriptorSetLayout> layouts = { mapDescriptor.GetDescriptorInfo().DescriptorSetLayout };

		auto pipelineRes = vk::CreateComputePipeline(*VulkanApp, cs, layouts);
		ASSERT(pipelineRes, "Couldn't create compute pipeline!");


		auto cmd = vk::BeginCommands(*VulkanApp, VulkanApp->CommandPoolCQ);

		vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_COMPUTE, pipelineRes->Handle);
		vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_COMPUTE, pipelineRes->Layout, 0, mapDescriptor.GetDescriptorInfo().DescriptorSets.size(),
			mapDescriptor.GetDescriptorInfo().DescriptorSets.data(), 0, 0);

		const int workGroups = 16;
		cs.Dispatch(cmd, hdrData.Width / workGroups, hdrData.Height / workGroups, 6);

		vk::EndCommands(*VulkanApp, VulkanApp->CommandPoolCQ, cmd, VulkanApp->ComputeQueue);


		cs.Cleanup();
		hdrTexture.Cleanup();
		mapDescriptor.Destroy();
		vk::DestoryPipeline(*VulkanApp, *pipelineRes);

		size_t newId = AM->IncrementImageCounter();
		TM.AddTexture(newId, map);

		return newId;
	}
}