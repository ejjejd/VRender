#pragma once
#include "vrender.h"

#include "assimp/Importer.hpp"
#include "assimp/scene.h"
#include "assimp/postprocess.h"

namespace asset
{
	struct MeshInfo
	{
		std::string Name;

		std::vector<glm::vec3> Positions;
		std::vector<glm::vec3> Normals;
		std::vector<glm::vec2> UVs;

		std::vector<glm::vec3> Tangents;
		std::vector<glm::vec3> Bitangents;
	};
}

namespace manager
{
	class AssetManager
	{
	private:
		Assimp::Importer AssimImporter;

		std::unordered_map<size_t, asset::MeshInfo> MeshesLookup;
	public:
		size_t LoadMeshInfo(const std::string& filepath);

		inline asset::MeshInfo GetMeshInfo(const size_t id)
		{
			auto findRes = MeshesLookup.find(id);
			if (findRes == MeshesLookup.end())
			{
				LOG("Error getting mesh under id: %d", id)
				return asset::MeshInfo{};
			}

			return findRes->second;
		}

		inline void UnloadAll()
		{
			MeshesLookup.clear();
		}
	};
}