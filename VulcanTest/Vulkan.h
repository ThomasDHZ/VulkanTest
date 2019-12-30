#pragma once
#include <vulkan/vulkan.h>

#include <iostream>
#include <stdexcept>
#include <functional>
#include <cstdlib>
#include "VulkanWindow.h"
#include "VulkanDebugger.h"
#include "QueueFamilyStruct.h"

class Vulkan
{ 
private:
	const bool EnableValidationLayers;
	std::vector<const char*> ValidationLayers;

	VulkanWindow Window;
	VkInstance VulkanInstance;
	VulkanDebugger VulkenDebug;
	VkPhysicalDevice VideoCardDevice;
	VkDevice Device;
	VkQueue GraphicsQueue;

	void CreateVulkanInstance();
	void SetUpDebugger();
	void SetUpVideoCard();
	void SetUpLogicalDevice();
	void MainLoop();

	QueueFamilyIndices QueueFamilies(VkPhysicalDevice device);
	bool VideoCardVulkanCompatible(VkPhysicalDevice device);
	bool ValidationLayerSupport();
	std::vector<const char*> GetRequiredExtensions();
public:
	Vulkan(unsigned int width, unsigned int height, const char* windowName);
	~Vulkan();
	void Run();
	void GetVulkanExtenstions();
};

