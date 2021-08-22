#include "asset_manager.h"

#include "assimp/Importer.hpp"
#include "assimp/scene.h"
#include "assimp/postprocess.h"

#include "vendors/stb/stb_image.h"

namespace manager
{
	asset::MeshInfo ConvertMesh(const aiMesh* assimpMesh)
	{
		asset::MeshInfo mesh;

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

	asset::AssetId AssetManager::LoadMeshInfo(const std::string& filepath)
	{
		Assimp::Importer assimpImporter;

		const auto scene = assimpImporter.ReadFile(filepath, aiProcess_Triangulate | aiProcess_CalcTangentSpace);

		if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
		{
			LOGE("Error loading mesh: %s", filepath);
			return -1;
		}


		size_t meshId = MeshesLookup.size();
		MeshesLookup[meshId] = ConvertMesh(scene->mMeshes[0]);
		
		assimpImporter.FreeScene();

		return meshId;
	}

	asset::AssetId AssetManager::LoadImageInfo(const std::string& filepath)
	{
		int texWidth, texHeight, texChannels;
		stbi_set_flip_vertically_on_load(1);
		auto pixels = stbi_load(filepath.c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);

		if (!pixels)
		{
			LOGE("Error loading image: %s", filepath);
			return -1;
		}

		asset::ImageInfo imageInfo;
		imageInfo.Width = texWidth;
		imageInfo.Height = texHeight;
		
		imageInfo.PixelsData.resize(texWidth * texHeight * 4);
		memcpy(&imageInfo.PixelsData[0], pixels, texWidth * texHeight * 4);

		size_t imageId = ImagesLookup.size();
		ImagesLookup[imageId] = imageInfo;

		stbi_image_free(pixels);

		return imageId;
	}
}