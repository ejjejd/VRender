#pragma once
#include "vrender.h"

namespace render
{
	struct PointLight
	{
		glm::vec3 Position;
		glm::vec3 Color;
	};

	struct Spotlight
	{
		glm::vec3 Position;
		glm::vec3 Direction;
		glm::vec3 Color;

		float OuterAngle;
		float InnerAngle;
	};
}