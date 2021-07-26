#pragma once
#include "vrender.h"
#include "graphics/shader.h"

#include "managers/asset_manager.h"

namespace render
{
	struct Mesh
	{
		std::string VertexShader;
		std::string FragmentShader;

		asset::MeshInfo MeshInfo;
	};
}