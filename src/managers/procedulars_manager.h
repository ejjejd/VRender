#pragma once
#include "vrender.h"
#include "vulkan/vulkan_app.h"

#include "asset_manager.h"

#include "vulkan/texture.h"
#include "vulkan/helpers.h"

namespace manager
{
	class ProceduralsManager
	{
	private:
		std::unordered_map<asset::AssetId, vk::Texture> ProceduralTextures;

		vk::VulkanApp* App;
		manager::AssetManager* AM;
	public:
		bool Setup(vk::VulkanApp& app, AssetManager& am)
		{
			App = &app;
			AM = &am;
		}

		void Cleanup()
		{

		}

		size_t GenerateIrradianceMap(const asset::AssetId id);
	};
}