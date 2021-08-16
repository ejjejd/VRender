#pragma once
#include <functional>

#include "vrender.h"

#include "vulkan/vulkan_app.h"

#include "managers/scene_manager.h"
#include "managers/render_manager.h"
#include "managers/asset_manager.h"
#include "managers/input_manager.h"

#include "utils/timer.h"

namespace app
{
	struct API Engine
	{
		vk::VulkanApp VulkanApp;

		manager::SceneManager SceneManager;
		manager::RenderManager RenderManager;
		manager::AssetManager AssetManager;
		manager::InputManager InputManager;

		uint16_t WindowWidth = 1280;
		uint16_t WindowHeight = 720;

		float DeltaTime = 0.0f;
		uint16_t Fps = 0.0f;

		inline void StartupEngine()
		{
			if (!vk::SetupVulkanApp(WindowWidth, WindowHeight, VulkanApp))
				return;

			if (!RenderManager.Setup(VulkanApp, AssetManager))
				return;

			SceneManager.Setup(RenderManager);

			InputManager.Setup(VulkanApp);
		}

		inline void CleanupEngine()
		{
			RenderManager.Cleanup();

			vk::CleanVulkanApp(VulkanApp);
		}

		inline void Run(const std::function<void()>& userMainLoop)
		{
			utils::Timer frameTimer;

			vk::RunVulkanApp(VulkanApp, 
				[&]()
				{
					frameTimer.Start();


					InputManager.Update();

					userMainLoop();

					SceneManager.Update();

					RenderManager.Update();


					Fps = 1000.0f / frameTimer.GetElapsedTime();
					DeltaTime = 1.0f / Fps;
				});
		}
	};
}