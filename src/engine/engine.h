#pragma once
#include <functional>

#include "vrender.h"

#include "vulkan/vulkan_app.h"

#include "managers/scene_manager.h"
#include "managers/render_manager.h"
#include "managers/asset_manager.h"
#include "managers/input_manager.h"
#include "debug/logger.h"

#include "utils/timer.h"

namespace app
{
	class StandardPrinter : public debug::BasePrinter
	{
	public:
		inline void OnReceive(const debug::LogSeverity severity, const std::string& message) override
		{
			printf(message.c_str());
		}

		inline std::string FormatMessage(const char* format, va_list args) override
		{
			va_list listCopy;

			va_copy(listCopy, args);
			size_t bufLength = vsnprintf(nullptr, 0, format, args);

			if (bufLength < 0)
				return {};

			va_end(listCopy);


			std::vector<char> buf(bufLength + 1);
			vsnprintf(&buf[0], bufLength + 1, format, args);

			return buf.data();
		}
	};

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
		float Fps = 0.0f;

		inline void StartupEngine()
		{
			if (!debug::GlobalLoggger.Setup("log.csv"))
			{
				printf("Couldn't initialize logger, probably problem with output file creation");
				return;
			}
			debug::GlobalLoggger.SetPrinter<StandardPrinter>();
			debug::GlobalLoggger.SetSpamSettings(2);


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

			debug::GlobalLoggger.Cleanup();
		}

		inline void Run(const std::function<void()>& userMainLoop)
		{
			utils::Timer frameTimer;

			vk::RunVulkanApp(VulkanApp, 
				[&]()
				{
					frameTimer.Start();


					debug::GlobalLoggger.Update();


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