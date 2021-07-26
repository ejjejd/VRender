#pragma once
#include "vrender.h"
#include "graphics/shader.h"

namespace render
{
	struct Mesh
	{
		std::string VertexShader;
		std::string FragmentShader;

		std::vector<glm::vec3> Positions;
	};
}