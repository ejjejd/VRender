#include "asset_manager.h"

#include "assimp/Importer.hpp"
#include "assimp/scene.h"
#include "assimp/postprocess.h"

#include "vendors/stb/stb_image.h"

namespace manager
{
	void AssetManager::LoadAssetsFromFolder(const std::string& path)
	{
		auto wstrPath = std::filesystem::path(path).wstring();

		auto relPath = std::filesystem::path::preferred_separator + wstrPath;
		auto p = std::filesystem::current_path() / wstrPath;

		auto strPath = std::filesystem::path(p).string();
		LOGC("Loading assets from %s...", strPath.c_str());

		LoadedDirectories.push_back(strPath);

		utils::Timer t;
		t.Start();

		for (auto& c : std::filesystem::recursive_directory_iterator(p))
		{
			if (c.is_directory())
				continue;

			auto str = c.path().string();
			if(!LoadSingleAsset(str))
				LOGE("Couldn't load asset: %s", str.c_str());
			else
				LOGC("Asset loaded: %s", str.c_str());
		}

		LOGC("Assets loaded in %fms", t.GetElapsedTime());
	}	

	template<typename T, typename T1>
	inline void MergeVector(std::vector<T>& v, const std::vector<T1>& v1)
	{
		v.insert(v.end(), v1.begin(), v1.end());
	}

	MeshData ConvertMesh(const aiMesh* assimpMesh)
	{
		MeshData mesh;

		bool haveTangentSpace = true;

		if (!assimpMesh->HasTangentsAndBitangents())
		{
			haveTangentSpace = false;
			LOGW("Can't generate tangent space for mesh: %s", assimpMesh->mName.C_Str());
		}

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

				if (haveTangentSpace)
				{
					auto tangent = assimpMesh->mTangents[id];
					auto bitangent = assimpMesh->mBitangents[id];

					mesh.Tangents.emplace_back(tangent.x, tangent.y, tangent.z);
					mesh.Bitangents.emplace_back(bitangent.x, bitangent.y, bitangent.z);
				}
			}
		}

		mesh.Name = assimpMesh->mName.C_Str();

		return mesh;
	}

	bool AssetManager::TryToLoadAsMesh(const utils::HashString& filepath)
	{
		Assimp::Importer assimpImporter;

		const auto scene = assimpImporter.ReadFile(filepath.GetString(), aiProcess_Triangulate | aiProcess_CalcTangentSpace);

		if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
			return false;

		MeshInfo info;
		info.NameOffset = MeshesData.Names.size();
		info.PositionsRDO.StartPosition = MeshesData.Positions.size();
		info.NormalsRDO.StartPosition = MeshesData.Normals.size();
		info.UvsRDO.StartPosition = MeshesData.UVs.size();
		info.TangentsRDO.StartPosition = MeshesData.Tangents.size();
		info.BitangentsRDO.StartPosition = MeshesData.Bitangents.size();


		auto meshData = ConvertMesh(scene->mMeshes[0]);

		MeshesData.Names.push_back(meshData.Name);
		MergeVector(MeshesData.Positions, meshData.Positions);
		MergeVector(MeshesData.Normals, meshData.Normals);
		MergeVector(MeshesData.UVs, meshData.UVs);
		MergeVector(MeshesData.Tangents, meshData.Tangents);
		MergeVector(MeshesData.Bitangents, meshData.Bitangents);


		info.PositionsRDO.EndPosition = MeshesData.Positions.size() - 1;
		info.NormalsRDO.EndPosition = MeshesData.Normals.size() - 1;
		info.UvsRDO.EndPosition = MeshesData.UVs.size() - 1;
		info.TangentsRDO.EndPosition = MeshesData.Tangents.size() - 1;
		info.BitangentsRDO.EndPosition = MeshesData.Bitangents.size() -1;

		MeshesOffsetLookup[filepath.GetHash()] = info;

		assimpImporter.FreeScene();

		return true;
	}

	bool AssetManager::TryToLoadAsImage(const utils::HashString& filepath)
	{
		int texWidth, texHeight, texChannels;
		stbi_set_flip_vertically_on_load(1);

		auto ext = filepath.GetString().substr(filepath.GetString().find_last_of(".") + 1);

		bool hdr = false;
		if (ext == "hdr")
			hdr = true;


		void* pixels = nullptr;
		if (!hdr)
			pixels = stbi_load(filepath.GetString().c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
		else
			pixels = stbi_loadf(filepath.GetString().c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);

		if (!pixels)
			return false;

		ImageData data;
		data.Width = texWidth;
		data.Height = texHeight;
		data.Hdr = hdr;
		
		data.PixelsData.resize(texWidth * texHeight * 4 * (hdr ? sizeof(float) : 1));
		memcpy(&data.PixelsData[0], pixels, texWidth * texHeight * 4 * (hdr ? sizeof(float) : 1));


		ImageInfo info;
		info.SizeOffset = ImagesData.Sizes.size();
		info.HdrStateOffset = ImagesData.HdrStates.size();
		info.PixelsRDO.StartPosition = ImagesData.Pixels.size();
		
		ImagesData.Sizes.push_back({ data.Width, data.Height });
		ImagesData.HdrStates.push_back(data.Hdr);
		MergeVector(ImagesData.Pixels, data.PixelsData);

		info.PixelsRDO.EndPosition = ImagesData.Pixels.size() - 1;

		ImagesOffsetLookup[filepath.GetHash()] = info;

		stbi_image_free(pixels);

		return true;
	}
}
