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
#include "Shader.h"

class Vulkan
{ 
private:
	const int MAX_FRAMES_IN_FLIGHT = 2;
	const bool EnableValidationLayers;
	std::vector<const char*> ValidationLayers;
	std::vector<const char*> DeviceExtensions;

	unsigned int Width;
	unsigned int Height;

	ShaderCompiler SCompiler;

	VulkanWindow Window;
	VkInstance VulkanInstance;
	VulkanDebugger VulkenDebug;
	VkPhysicalDevice VideoCardDevice;
	VkDevice Device;
	VkQueue GraphicsQueue;
	VkSurfaceKHR WindowSurface;
	VkQueue PresentationQueue;
	VkSwapchainKHR SwapChain;
	VkFormat SwapChainImageFormat;
	VkExtent2D SwapChainExtention;
	VkRenderPass RenderPass;
	VkPipelineLayout PipelineLayout;
	VkPipeline GraphicsPipeline;
	VkCommandPool CommandPool;

	std::vector<VkImage> SwapChainImages;
	std::vector<VkImageView> SwapChainImageViews;
	std::vector<VkFramebuffer> SwapChainFramebuffers;
	std::vector<VkCommandBuffer> CommandBuffers;
	std::vector<VkSemaphore> ImageAvailableSemaphore;
	std::vector<VkSemaphore> RenderFinishedSemaphore;
	std::vector<VkFence> inFlightFences;
	std::vector<VkFence> imagesInFlight;

	size_t CurrentFrame = 0;

	void SetUpVulkanInstance();
	void SetUpDebugger();
	void SetUpWindowSurface();
	void SetUpVideoCard();
	void SetUpLogicalDevice();
	void SetUpSwapChain();
	void SetUpImageView();
	void SetUpRenderPass();
	void SetUpGraphicsPipline();
	void SetUpFrameBuffer();
	void SetUpCommandPool();
	void SetUpCommandBuffers();
	void SetUpSyncObjects();

	void Draw();
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

