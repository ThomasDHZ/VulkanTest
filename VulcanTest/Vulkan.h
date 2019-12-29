#pragma once
#include <vulkan/vulkan.h>

#include <iostream>
#include <stdexcept>
#include <functional>
#include <cstdlib>
#include "VulkanWindow.h"

class Vulkan
{ 
private:
	const bool EnableValidationLayers;
	std::vector<const char*> ValidationLayers;

	VulkanWindow Window;
	VkInstance VulkanInstance;
	VkDebugUtilsMessengerEXT debugMessenger;

	void CreateVulkanInstance();
	void MainLoop();

	void CreateDebugMessengerInfo(VkDebugUtilsMessengerCreateInfoEXT& DebugInfo);
	void SetUpDebugger();
	bool ValidationLayerSupport();
	std::vector<const char*> GetRequiredExtensions();

	static VKAPI_ATTR VkBool32 VKAPI_CALL DebugCallBack(VkDebugUtilsMessageSeverityFlagBitsEXT MessageSeverity, VkDebugUtilsMessageTypeFlagsEXT MessageType, const VkDebugUtilsMessengerCallbackDataEXT* CallBackData, void* UserData);
public:
	Vulkan(unsigned int width, unsigned int height, const char* windowName);
	~Vulkan();
	void Run();
	void GetVulkanExtenstions();
};

