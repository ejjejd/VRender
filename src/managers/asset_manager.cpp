#include "asset_manager.h"

namespace manager
{
	asset::MeshInfo ConvertMesh(const aiMesh* assimpMesh)
	{
		asset::MeshInfo mesh;

		for (size_t i = 0; i < assimpMesh->mNumVertices; ++i)
		{
			auto av = assimpMesh->mVertices[i];
			mesh.Positions.emplace_back(av.x, av.y, av.z);
		}

		return mesh;
	}

	size_t AssetManager::LoadMeshInfo(const std::string& filepath)
	{
		const auto scene = AssimImporter.ReadFile(filepath, aiProcess_Triangulate);

		if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
		{
			LOG("Error loading mesh: %s", filepath)
			return -1;
		}


		size_t meshId = MeshesLookup.size();

		std::vector<asset::MeshInfo> subMeshes;

		MeshesLookup[meshId] = ConvertMesh(scene->mMeshes[0]);
		
		AssimImporter.FreeScene();

		return meshId;
	}
}