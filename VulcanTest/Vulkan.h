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
#include <glm/glm.hpp>
#include <array>

struct Vertex 
{
	glm::vec2 pos;
	glm::vec3 color;

	static VkVertexInputBindingDescription GetBindingDescription()
	{
		VkVertexInputBindingDescription BindingDescription = {};
		BindingDescription.binding = 0;
		BindingDescription.stride = sizeof(Vertex);
		BindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
		return BindingDescription;
	}

	static std::array<VkVertexInputAttributeDescription, 2> GetAttributeDescriptions() 
	{
		std::array<VkVertexInputAttributeDescription, 2> AttributeDescriptions = {};

		AttributeDescriptions[0].binding = 0;
		AttributeDescriptions[0].location = 0;
		AttributeDescriptions[0].format = VK_FORMAT_R32G32_SFLOAT;
		AttributeDescriptions[0].offset = offsetof(Vertex, pos);

		AttributeDescriptions[1].binding = 0;
		AttributeDescriptions[1].location = 1;
		AttributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
		AttributeDescriptions[1].offset = offsetof(Vertex, color);

		return AttributeDescriptions;
	}
};

struct UniformBufferObject
{
	glm::mat4 model;
	glm::mat4 view;
	glm::mat4 proj;
};

const std::vector<Vertex> vertices = {

	{{-0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}},
	{{0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}},
	{{0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}},
	{{-0.5f, 0.5f}, {1.0f, 1.0f, 1.0f}}
};

const std::vector<uint16_t> indices = 
{
	0, 1, 2, 2, 3, 0
};

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
	VkDescriptorSetLayout DescriptorSetLayout;
	VkPipelineLayout PipelineLayout;
	VkPipeline GraphicsPipeline;
	VkCommandPool CommandPool;
	VkBuffer VertexBuffer;
	VkDeviceMemory VertexBufferMemory;
	VkBuffer indexBuffer;
	VkDeviceMemory indexBufferMemory;


	std::vector<VkImage> SwapChainImages;
	std::vector<VkImageView> SwapChainImageViews;
	std::vector<VkFramebuffer> SwapChainFramebuffers;
	std::vector<VkCommandBuffer> CommandBuffers;
	std::vector<VkSemaphore> ImageAvailableSemaphore;
	std::vector<VkSemaphore> RenderFinishedSemaphore;
	std::vector<VkFence> inFlightFences;
	std::vector<VkFence> imagesInFlight;
	std::vector<VkBuffer> uniformBuffers;
	std::vector<VkDeviceMemory> uniformBuffersMemory;

	size_t CurrentFrame = 0;

	void SetUpVulkanInstance();
	void SetUpDebugger();
	void SetUpWindowSurface();
	void SetUpVideoCard();
	void SetUpLogicalDevice();
	void SetUpSwapChain();
	void SetUpImageView();
	void SetUpRenderPass();
	void SetUpDescriptorSetLayout();
	void SetUpGraphicsPipline();
	void SetUpFrameBuffer();
	void SetUpCommandPool();
	void SetUpVertexBuffer();
	void SetUpIndexBuffer();
	void SetUpCommandBuffers();
	void SetUpSyncObjects();

	void RecreateSwapChain();
	void CleanUpSwapChain();

	void Draw();
	void MainLoop();

	void CreateBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory);
	void CopyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);

	QueueFamilyIndices QueueFamilies(VkPhysicalDevice device);
	bool VideoCardVulkanCompatible(VkPhysicalDevice device);
	bool VideoCardExtenstionSupport(VkPhysicalDevice device);
	bool ValidationLayerSupport();
	SwapChainSupportDetails CheckSwapChainSupport(VkPhysicalDevice device);
	VkSurfaceFormatKHR SwapChainFormat(const std::vector<VkSurfaceFormatKHR>& AvailableFormatList);
	VkPresentModeKHR SwapChainPresentation(const std::vector<VkPresentModeKHR>& AvailablePresentModeList);
	VkExtent2D SwapChainExtent(const VkSurfaceCapabilitiesKHR& capabilities);
	uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);

	std::vector<const char*> GetRequiredExtensions();
public:
	Vulkan(unsigned int width, unsigned int height, const char* windowName);
	~Vulkan();
	void Run();
	void GetVulkanExtenstions();
};

