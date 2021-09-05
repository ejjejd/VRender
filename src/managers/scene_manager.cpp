#include "scene_manager.h"

namespace manager
{
	void SceneManager::Update()
	{
		if(Cameras.size() >= 0)
			RM->SetActiveCamera(Cameras[ActiveCameraId]);

		if (RootNode)
		{
			auto meshV = RootNode->GetNodesWithChannel<scene::MeshRenderable>(scene::NodeChannel::MeshRenderable);
			RM->UpdateMeshUBO(meshV);

			auto plV = RootNode->GetNodesWithChannel<scene::PointLight>(scene::NodeChannel::PointLight);
			auto slV = RootNode->GetNodesWithChannel<scene::Spotlight>(scene::NodeChannel::Spotlight);
			RM->UpdateLightUBO(plV, slV);
		}
	}
}