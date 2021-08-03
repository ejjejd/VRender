 #pragma once
#include "vrender.h"
#include "vulkan/shader.h"

#include "managers/asset_manager.h"

#include "material.h"

namespace render
{
	struct MeshTransform
	{
		glm::vec3 Translate = glm::vec3(0.0f);
		glm::vec3 Scale = glm::vec3(1.0f);
		glm::vec4 Rotation = glm::vec4(glm::vec3(1.0f, 1.0f, 1.0f), 0.0f);
	};

	struct Mesh
	{
		std::shared_ptr<BaseMaterial> Material;

		asset::MeshInfo MeshInfo;

		MeshTransform Transform;
	};
}