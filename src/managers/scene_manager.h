#pragma once
#include "vrender.h"
#include "render_manager.h"

#include "rendering/mesh.h"
#include "rendering/light.h"

#include "graphics/camera.h"
#include "graphics/texture.h"

namespace manager
{
	class SceneManager
	{
	private:
		std::vector<std::reference_wrapper<render::Mesh>> RegisteredMeshes;

		std::vector<std::reference_wrapper<graphics::Camera>> Cameras;

		std::vector<std::reference_wrapper<render::PointLight>> RegisteredPointLights;
		std::vector<std::reference_wrapper<render::Spotlight>> RegisteredSpotlights;

		size_t ActiveCameraId = 0;

		RenderManager* RM;
	public:
		void Update();

		inline void Setup(RenderManager& rm)
		{
			RM = &rm;
		}

		inline void Register(render::Mesh& mesh)
		{
			RegisteredMeshes.push_back(mesh);
			RM->RegisterMesh(mesh);
		}

		inline void Register(render::PointLight& pl)
		{
			RegisteredPointLights.push_back(pl);
		}

		inline void Register(render::Spotlight& sl)
		{
			RegisteredSpotlights.push_back(sl);
		}

		inline size_t Register(graphics::Camera& camera)
		{
			Cameras.push_back(camera);
			return Cameras.size() - 1;
		}

		inline void SetActiveCamera(const size_t id)
		{
			if (id >= Cameras.size())
			{
				LOG("Invalid camera selected!")
				return;
			}

			ActiveCameraId = 0;
		}
	};
}