#include "engine.h"

namespace app
{
	std::string StandardPrinter::FormatMessage(const char* format, va_list args)
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

	void Engine::PrintPlatformInfo()
	{
		uint32_t majorApi = VK_API_VERSION_MAJOR(VulkanApp.DeviceProperties.apiVersion);
		uint32_t minorApi = VK_API_VERSION_MINOR(VulkanApp.DeviceProperties.apiVersion);
		uint32_t patchApi = VK_API_VERSION_PATCH(VulkanApp.DeviceProperties.apiVersion);

		LOGC("Device: %s", VulkanApp.DeviceProperties.deviceName);
		LOGC("Vulkan version: %d.%d.%d", majorApi, minorApi, patchApi);
	}

	void Engine::StartupEngine()
	{
		if (!debug::GlobalLoggger.Setup("log.txt"))
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


		PrintPlatformInfo();
	}

	void Engine::CleanupEngine()
	{
		RenderManager.Cleanup();

		vk::CleanVulkanApp(VulkanApp);

		debug::GlobalLoggger.Cleanup();
	}

	void Engine::Run(const std::function<void()>& userMainLoop)
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
}