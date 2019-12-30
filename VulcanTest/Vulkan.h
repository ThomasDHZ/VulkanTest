#pragma once
#include <vulkan/vulkan.h>

#include <iostream>
#include <stdexcept>
#include <functional>
#include <cstdlib>
#include "VulkanWindow.h"
#include "VulkanDebugger.h"
#include "QueueFamilyStruct.h"
#include "SwapChainSupportDetailsStruct.h"

class Vulkan
{ 
private:
	const bool EnableValidationLayers;
	std::vector<const char*> ValidationLayers;
	std::vector<const char*> DeviceExtensions;

	unsigned int Width;
	unsigned int Height;

	VulkanWindow Window;
	VkInstance VulkanInstance;
	VulkanDebugger VulkenDebug;
	VkPhysicalDevice VideoCardDevice;
	VkDevice Device;
	VkQueue GraphicsQueue;
	VkSurfaceKHR WindowSurface;
	VkQueue PresentationQueue;
	VkSwapchainKHR SwapChain;
	std::vector<VkImage> SwapChainImages;
	VkFormat SwapChainImageFormat;
	VkExtent2D SwapChainExtention;

	void SetUpVulkanInstance();
	void SetUpDebugger();
	void SetUpWindowSurface();
	void SetUpVideoCard();
	void SetUpLogicalDevice();
	void SetUpSwapChain();
	void MainLoop();

	QueueFamilyIndices QueueFamilies(VkPhysicalDevice device);
	bool VideoCardVulkanCompatible(VkPhysicalDevice device);
	bool VideoCardExtenstionSupport(VkPhysicalDevice device);
	bool ValidationLayerSupport();
	SwapChainSupportDetails CheckSwapChainSupport(VkPhysicalDevice device);
	VkSurfaceFormatKHR SwapChainFormat(const std::vector<VkSurfaceFormatKHR>& AvailableFormatList);
	VkPresentModeKHR SwapChainPresentation(const std::vector<VkPresentModeKHR>& AvailablePresentModeList);
	VkExtent2D SwapChainExtent(const VkSurfaceCapabilitiesKHR& capabilities);

	std::vector<const char*> GetRequiredExtensions();
public:
	Vulkan(unsigned int width, unsigned int height, const char* windowName);
	~Vulkan();
	void Run();
	void GetVulkanExtenstions();
};

