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
	class StandardPrinter : public debug::BasePrinter
	{
	public:
		inline void OnReceive(const debug::LogSeverity severity, const std::string& message) override
		{
			printf(message.c_str());
		}

		std::string FormatMessage(const char* format, va_list args) override;
	};

	class API Engine
	{
	private:
		void PrintPlatformInfo();
	public:
		vk::VulkanApp VulkanApp;

		manager::SceneManager SceneManager;
		manager::RenderManager RenderManager;
		manager::AssetManager AssetManager;
		manager::InputManager InputManager;

		uint16_t WindowWidth = 1280;
		uint16_t WindowHeight = 720;

		float DeltaTime = 0.0f;
		float Fps = 0.0f;

		void StartupEngine();

		void CleanupEngine();

		void Run(const std::function<void()>& userMainLoop);
	};
}