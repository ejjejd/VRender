#pragma once
#include "vrender.h"

#include "mesh.h"

namespace render
{
	struct Renderable
	{
		
		VkPipelineLayout GraphicsPipelineLayout;
		VkPipeline GraphicsPipeline;

		std::vector<VkCommandBuffer> CommandBuffers;
	};
}