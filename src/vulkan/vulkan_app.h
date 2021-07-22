#pragma once
#include "vrender.h"

#include <functional>

namespace vk
{
	struct VulkanApp
	{

	};

	void API SetupVulkanApp(VulkanApp& app);
	void API CleanVulkanApp(VulkanApp& app);

	void API RunVulkanApp(VulkanApp& app, std::function<void()>& callback);
}