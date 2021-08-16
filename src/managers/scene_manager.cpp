#include "scene_manager.h"

namespace manager
{
	void SceneManager::Update()
	{
		if(Cameras.size() >= 0)
			RM->SetActiveCamera(Cameras[ActiveCameraId]);

		RM->UpdateMeshUBO(RegisteredMeshes);
		RM->UpdateLightUBO(RegisteredPointLights, RegisteredSpotlights);
	}
}