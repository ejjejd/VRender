#pragma once
#include "vrender.h"
#include "render_manager.h"

#include "scene/scene_hi.h"
#include "rendering/camera.h"

#include "vulkan/texture.h"

namespace manager
{
	class SceneManager
	{
	private:
		std::vector<std::reference_wrapper<render::Camera>> Cameras;
		size_t ActiveCameraId = 0;

		scene::Node* RootNode;

		RenderManager* RM;
	public:
		inline void Setup(RenderManager& rm)
		{
			RM = &rm;
		}

		void Update();

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

		inline void SetRoot(scene::Node* node)
		{
			RootNode = node;

			auto meshV = RootNode->GetNodesWithChannel<scene::MeshRenderable>();
			for (auto& m : meshV)
				RM->RegisterMesh(m);
		}
	};
}