#include "vulkan/vulkan_app.h"

constexpr uint16_t WindowWidth = 1280;
constexpr uint16_t WindowHeight = 720;

int main()
{
	vk::VulkanApp vkApp;

	if (!vk::SetupVulkanApp(WindowWidth, WindowHeight, vkApp))
		return -1;

	vk::RunVulkanApp(vkApp, 
		[]()
		{

		});

	vk::CleanVulkanApp(vkApp);

	return 0;
}