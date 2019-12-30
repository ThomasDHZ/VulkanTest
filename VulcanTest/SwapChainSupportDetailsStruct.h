#pragma once
#include <vulkan\vulkan_core.h>
#include <vector>

struct SwapChainSupportDetails 
{
	VkSurfaceCapabilitiesKHR Capabilities;
	std::vector<VkSurfaceFormatKHR> Formats;
	std::vector<VkPresentModeKHR> PresentModes;
};