#pragma once
#include "vrender.h"

#include <vector>
#include <optional>
#include <filesystem>

namespace utils
{
	class HashString
	{
	private:
		size_t HashValue = -1;
		std::string StringValue;
	public:
		HashString() = default;
		~HashString() = default;

		inline HashString(const std::string& str)
			: StringValue(str)
		{
			HashValue = std::hash<std::string>{}(str);
		}

		inline HashString(const char* str)
			: StringValue(str)
		{
			HashValue = std::hash<const char*>{}(str);
		}

		inline void operator = (const std::string& str)
		{
			StringValue = str;
			HashValue = std::hash<std::string>{}(str);
		}

		inline bool operator == (const HashString& hs)
		{
			return (HashValue == hs.GetHash());
		}

		inline std::string GetString() const
		{
			return StringValue;
		}

		inline size_t GetHash() const
		{
			return HashValue;
		}

		HashString(HashString&&) = default;
		HashString(const HashString&) = default;
		HashString& operator = (HashString&&) = default;
		HashString& operator = (const HashString&) = default;
	};
}

//Convert engine path to platform dependent
inline std::string operator"" _ep(const char* path, const size_t length)
{
	auto wstrPath = std::filesystem::path(path).wstring();
	for (auto& c : wstrPath) //Convert path to one format
	{
		if (c == '\\' || c == '/')
			c = std::filesystem::path::preferred_separator;
	}

	return std::filesystem::path(wstrPath).string();
}

namespace manager
{
	using AssetId = size_t;

    struct RangeDataOffset
	{
		size_t StartPosition;
		size_t EndPosition;
	};

	struct MeshInfo
	{
		size_t NameOffset;

		RangeDataOffset PositionsRDO;
		RangeDataOffset NormalsRDO;
		RangeDataOffset UvsRDO;

		RangeDataOffset TangentsRDO;
		RangeDataOffset BitangentsRDO;
	};

	struct ImageInfo
	{
		size_t SizeOffset;

		RangeDataOffset PixelsRDO;

		size_t HdrStateOffset;
	};

    struct MeshData
	{
		std::string Name;

		std::vector<glm::vec3> Positions;
		std::vector<glm::vec3> Normals;
		std::vector<glm::vec2> UVs;

		std::vector<glm::vec3> Tangents;
		std::vector<glm::vec3> Bitangents;
	};

	struct ImageData
	{
		uint16_t Width;
		uint16_t Height;

		std::vector<float> PixelsData;

		bool Hdr;
	};

	class AssetManager
	{
	private:
		struct
		{
			std::vector<std::string> Names;

			std::vector<glm::vec3> Positions;
			std::vector<glm::vec3> Normals;
			std::vector<glm::vec2> UVs;
			
			std::vector<glm::vec3> Tangents;
			std::vector<glm::vec3> Bitangents;

            inline void ClearAll()
            {
                Names.clear();

                Positions.clear();
                Normals.clear();
                UVs.clear();

                Tangents.clear();
                Bitangents.clear();
            }
		} MeshesData;

		struct
		{
			std::vector<glm::ivec2> Sizes;

			std::vector<float> Pixels;

			std::vector<bool> HdrStates;

            inline void ClearAll()
            {
                Sizes.clear();

                Pixels.clear();

                HdrStates.clear();
            }
		} ImagesData;

		std::unordered_map<AssetId, MeshInfo> MeshesOffsetLookup;
		std::unordered_map<AssetId, ImageInfo> ImagesOffsetLookup;

		std::vector<std::string> LoadedDirectories;

		bool TryToLoadAsMesh(const utils::HashString& filepath);
		bool TryToLoadAsImage(const utils::HashString& filepath);
	public:
		void LoadAssetsFromFolder(const std::string& path);

		inline bool LoadSingleAsset(const utils::HashString& path)
		{
			if (TryToLoadAsMesh(path))
				return true;
			else if (TryToLoadAsImage(path))
				return true;

			return false;
		}

		inline void UnloadAll()
		{
            MeshesOffsetLookup.erase(MeshesOffsetLookup.begin(), MeshesOffsetLookup.end());
            ImagesOffsetLookup.erase(ImagesOffsetLookup.begin(), ImagesOffsetLookup.end());

            MeshesData.ClearAll();
            ImagesData.ClearAll();
		}

        inline MeshInfo GetMeshData(const utils::HashString& filepath)
		{
			for (const auto& pd : LoadedDirectories)
			{
				utils::HashString hs = (std::filesystem::path(pd) 
										/ std::filesystem::path(filepath.GetString())).string();

				auto findRes = MeshesOffsetLookup.find(hs.GetHash());
				if (findRes != MeshesOffsetLookup.end())
					return findRes->second;
			}

			LOGE("Error getting mesh info under path: %s", filepath.GetString().c_str());
			return MeshInfo{};
		}

		inline ImageInfo GetImageData(const utils::HashString& filepath)
		{
			for (const auto& pd : LoadedDirectories)
			{
				utils::HashString hs = (std::filesystem::path(pd)
										/ std::filesystem::path(filepath.GetString())).string();

				auto findRes = ImagesOffsetLookup.find(hs.GetHash());
				if (findRes != ImagesOffsetLookup.end())
					return findRes->second;
			}

			LOGE("Error getting image info under path: %s", filepath.GetString().c_str());
			return ImageInfo{};
		}

		inline bool IsMeshLoaded(const AssetId id)
		{
			auto findRes = MeshesOffsetLookup.find(id);
			return (findRes != MeshesOffsetLookup.end());
		}

		inline bool IsImageLoaded(const AssetId id)
		{
			auto findRes = ImagesOffsetLookup.find(id);
			return (findRes != ImagesOffsetLookup.end());
		}

		AssetId LoadMeshInfo(const std::string& filepath)
		{
			return {};
		}

		AssetId LoadImageInfo(const std::string& filepath)
		{
			return {};
		}	


		//This functions needed for procedural content

		inline size_t IncerementMeshCounter()
		{
			return {};
		}

		inline size_t IncrementImageCounter()
		{
			return {};
		}
	};
}
