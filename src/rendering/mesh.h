 #pragma once
#include "vrender.h"
#include "vulkan/shader.h"

#include "managers/asset_manager.h"

namespace render
{
	struct MeshTransform
	{
		glm::vec3 Translate;
		glm::vec3 Scale;
		glm::vec4 Rotation;
	};

	struct Mesh
	{
		std::string VertexShader;
		std::string FragmentShader;

		asset::MeshInfo MeshInfo;

		MeshTransform Transform;
	};
}