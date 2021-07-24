#pragma once
#include <functional>

#include "vrender.h"

#include "vulkan/vulkan_app.h"

#include "managers/scene_manager.h"
#include "managers/render_manager.h"

namespace app
{
	struct API Engine
	{
		vk::VulkanApp VulkanApp;

		manager::SceneManager SceneManager;
		manager::RenderManager RenderManager;

		uint16_t WindowWidth = 1280;
		uint16_t WindowHeight = 720;

		inline void StartupEngine()
		{
			vk::SetupVulkanApp(WindowWidth, WindowHeight, VulkanApp);

			RenderManager.Setup(VulkanApp);
		}

		inline void CleanupEngine()
		{
			RenderManager.Cleanup(VulkanApp);

			vk::CleanVulkanApp(VulkanApp);
		}

		inline void Run(const std::function<void()>& userMainLoop)
		{
			vk::RunVulkanApp(VulkanApp, 
				[&]()
				{
					userMainLoop();

					RenderManager.Update(VulkanApp);
				});
		}
	};
}