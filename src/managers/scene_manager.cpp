#include "scene_manager.h"

namespace manager
{
	void SceneManager::Update()
	{
		if(Cameras.size() >= 0)
			RM->SetActiveCamera(Cameras[ActiveCameraId]);

		if (RootNode)
		{
			auto meshV = RootNode->GetNodesWithChannel<scene::MeshRenderable>();
			RM->UpdateMeshUBO(meshV);

			auto plV = RootNode->GetNodesWithChannel<scene::PointLight>();
			auto slV = RootNode->GetNodesWithChannel<scene::Spotlight>();
			RM->UpdateLightUBO(plV, slV);
		}
	}
}