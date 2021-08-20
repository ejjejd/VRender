#pragma once
#include "vrender.h"
#include "vulkan/shader.h"

#include "managers/asset_manager.h"

#include "material.h"

namespace scene
{
	inline size_t GlobalId = 0;

	class SceneObject
	{
	private:
		size_t Id = GlobalId++;
	public:
		inline size_t GetId() const
		{
			return Id;
		}
	};

	struct RenderInfo
	{
		VkCompareOp DepthCompareOP;
	};

	class RenderObject : public SceneObject
	{
	public:
		RenderInfo Info;
	};

	struct MeshTransform
	{
		glm::vec3 Translate = glm::vec3(0.0f);
		glm::vec3 Scale = glm::vec3(1.0f);
		glm::vec4 Rotation = glm::vec4(glm::vec3(1.0f, 1.0f, 1.0f), 0.0f);
	};

	class Mesh : public RenderObject
	{
	public:
		inline Mesh()
		{
			Info.DepthCompareOP = VK_COMPARE_OP_LESS;
		}

		asset::MeshInfo MeshInfo;

		MeshTransform Transform;

		std::shared_ptr<render::BaseMaterial> Material;
	};

	class PointLight : public SceneObject
	{
	public:
		glm::vec3 Position;
		glm::vec3 Color;
	};

	class Spotlight : public SceneObject
	{
	public:
		glm::vec3 Position;
		glm::vec3 Direction;
		glm::vec3 Color;

		float OuterAngle;
		float InnerAngle;
	};
}