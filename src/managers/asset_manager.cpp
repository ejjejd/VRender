#include "asset_manager.h"

namespace manager
{
	asset::MeshInfo ConvertMesh(const aiMesh* assimpMesh)
	{
		asset::MeshInfo mesh;

		for (size_t i = 0; i < assimpMesh->mNumFaces; ++i)
		{
			auto face = assimpMesh->mFaces[i];

			for (size_t j = 0; j < face.mNumIndices; ++j)
			{
				auto id = face.mIndices[j];

				auto av = assimpMesh->mVertices[id];
				mesh.Positions.emplace_back(av.x, av.y, av.z);

				auto nv = assimpMesh->mNormals[id];
				mesh.Normals.emplace_back(nv.x, nv.y, nv.z);

				if (assimpMesh->mTextureCoords[0])
				{
					auto uv = assimpMesh->mTextureCoords[0][id];
					mesh.UVs.emplace_back(uv.x, uv.y);
				}
			}
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