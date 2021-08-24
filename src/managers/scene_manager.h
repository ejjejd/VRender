#pragma once
#include "vrender.h"
#include "render_manager.h"

#include "rendering/scene_objects.h"
#include "rendering/camera.h"

#include "vulkan/texture.h"

namespace manager
{
	class SceneManager
	{
	private:
		std::vector<std::reference_wrapper<scene::Mesh>> RegisteredMeshes;

		std::vector<std::reference_wrapper<render::Camera>> Cameras;

		std::vector<std::reference_wrapper<scene::PointLight>> RegisteredPointLights;
		std::vector<std::reference_wrapper<scene::Spotlight>> RegisteredSpotlights;

		size_t ActiveCameraId = 0;

		RenderManager* RM;
	public:
		void Update();

		inline void Setup(RenderManager& rm)
		{
			RM = &rm;
		}

		inline void Register(scene::Mesh& mesh)
		{
			RegisteredMeshes.push_back(mesh);
			RM->RegisterMesh(mesh);
		}

		inline void Register(scene::PointLight& pl)
		{
			RegisteredPointLights.push_back(pl);
		}

		inline void Register(scene::Spotlight& sl)
		{
			RegisteredSpotlights.push_back(sl);
		}

		inline size_t Register(render::Camera& camera)
		{
			Cameras.push_back(camera);
			return Cameras.size() - 1;
		}

		inline void SetActiveCamera(const size_t id)
		{
			if (id >= Cameras.size())
			{
				LOGE("Invalid camera selected!");
				return;
			}

			ActiveCameraId = 0;
		}
	};
}