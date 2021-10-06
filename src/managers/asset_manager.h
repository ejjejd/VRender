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

		inline HashString(const size_t hashId)
			: HashValue(hashId) {}

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

	struct RangeDataOffset
	{
		size_t StartPosition;
		size_t EndPosition;
	};

	template<typename T, typename T1>
	inline void MergeVector(std::vector<T>& v, const std::vector<T1>& v1)
	{
		v.insert(v.end(), v1.begin(), v1.end());
	}

	template<typename T, typename T1>
	inline void MergeVector(std::vector<T>& v, const std::vector<T1>& v1, const RangeDataOffset rdo)
	{
		v.insert(v.end(), v1.begin() + rdo.StartPosition, v1.begin() + rdo.EndPosition);
	}
}

//Convert engine path to platform dependent
inline const char* operator"" _ep(const char* path, const size_t length)
{
	char* str = new char[length + 1];

	for (size_t i = 0; i <= length; ++i)
	{
		if (path[i] == '/' || path[i] == '\\')
			str[i] = std::filesystem::path::preferred_separator; //Assume that the seperator character have the same code in different charsets
		else
			str[i] = path[i];
	}

	return str;
}

namespace manager
{
	using AssetId = size_t;

	struct MeshInfo
	{
		size_t NameOffset;

		utils::RangeDataOffset PositionsRDO;
		utils::RangeDataOffset NormalsRDO;
		utils::RangeDataOffset UvsRDO;

		utils::RangeDataOffset TangentsRDO;
		utils::RangeDataOffset BitangentsRDO;
	};

	struct ImageInfo
	{
		size_t SizeOffset;

		utils::RangeDataOffset PixelsRDO;

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

		size_t ProcIdsCount = 0;

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

        inline MeshData GetMeshData(const utils::HashString& filepath)
		{
			for (const auto& pd : LoadedDirectories)
			{
				utils::HashString hs = (std::filesystem::path(pd) 
										/ std::filesystem::path(filepath.GetString())).string();

				auto findRes = MeshesOffsetLookup.find(hs.GetHash());
				if (findRes != MeshesOffsetLookup.end())
				{
					auto& info = findRes->second;

					MeshData data;
					data.Name = MeshesData.Names[info.NameOffset];
					utils::MergeVector(data.Positions, MeshesData.Positions, info.PositionsRDO);
					utils::MergeVector(data.Normals, MeshesData.Normals, info.NormalsRDO);
					utils::MergeVector(data.UVs, MeshesData.UVs, info.UvsRDO);
					utils::MergeVector(data.Tangents, MeshesData.Tangents, info.TangentsRDO);
					utils::MergeVector(data.Bitangents, MeshesData.Bitangents, info.BitangentsRDO);

					return data;
				}
			}

			LOGE("Error getting mesh data under path: %s", filepath.GetString().c_str());
			return {};
		}

		inline ImageData GetImageData(const utils::HashString& filepath)
		{
			for (const auto& pd : LoadedDirectories)
			{
				utils::HashString hs = (std::filesystem::path(pd)
										/ std::filesystem::path(filepath.GetString())).string();

				auto findRes = ImagesOffsetLookup.find(hs.GetHash());
				if (findRes != ImagesOffsetLookup.end())
				{
					auto& info = findRes->second;

					ImageData data;
					data.Width = ImagesData.Sizes[info.SizeOffset].x;
					data.Height = ImagesData.Sizes[info.SizeOffset].y;
					data.Hdr = ImagesData.HdrStates[info.HdrStateOffset];
					utils::MergeVector(data.PixelsData, ImagesData.Pixels, info.PixelsRDO);

					return data;
				}
			}

			LOGE("Error getting image data under path: %s", filepath.GetString().c_str());
			return {};
		}

		inline bool IsMeshLoaded(const utils::HashString& filepath)
		{
			for (const auto& pd : LoadedDirectories)
			{
				utils::HashString hs = (std::filesystem::path(pd)
										/ std::filesystem::path(filepath.GetString())).string();

				auto findRes = MeshesOffsetLookup.find(hs.GetHash());
				if (findRes != MeshesOffsetLookup.end())
					return true;
			}

			return false;
		}

		inline bool IsImageLoaded(const utils::HashString& filepath)
		{
			for (const auto& pd : LoadedDirectories)
			{
				utils::HashString hs = (std::filesystem::path(pd)
					/ std::filesystem::path(filepath.GetString())).string();

				auto findRes = ImagesOffsetLookup.find(hs.GetHash());
				if (findRes != ImagesOffsetLookup.end())
					return true;
			}

			return false;
		}

		//This functions needed for procedural content
		inline AssetId GetProcId()
		{
			return (SIZE_MAX - ProcIdsCount++);
		}
	};
}
