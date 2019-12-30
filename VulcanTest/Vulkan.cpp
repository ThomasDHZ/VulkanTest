#include "Vulkan.h"
#include <set>
#include <algorithm> 

Vulkan::Vulkan(unsigned int width, unsigned int height, const char* windowName) : EnableValidationLayers(true)
{
	VideoCardDevice = VK_NULL_HANDLE;

	Window = VulkanWindow(width, height, windowName);

	ValidationLayers.emplace_back("VK_LAYER_KHRONOS_validation");
	DeviceExtensions.emplace_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);

	SetUpVulkanInstance();
	SetUpDebugger();
	SetUpWindowSurface();
	SetUpVideoCard();
	SetUpLogicalDevice();
	SetUpSwapChain();
}

Vulkan::~Vulkan()
{
	if (EnableValidationLayers)
	{
		VulkenDebug.DestroyDebugUtilsMessengerEXT(VulkanInstance, nullptr);
	}
	vkDestroySwapchainKHR(Device, SwapChain, nullptr);
	vkDestroyDevice(Device, nullptr);
	vkDestroySurfaceKHR(VulkanInstance, WindowSurface, nullptr);
	vkDestroyInstance(VulkanInstance, nullptr);
	glfwDestroyWindow(Window.GetWindowPtr());
	glfwTerminate();
}

void Vulkan::SetUpVulkanInstance()
{
	if (EnableValidationLayers && !ValidationLayerSupport())
	{
		throw std::runtime_error("validation layers requested, but not available!");
	}

	unsigned int glfwExtensionCount = 0;
	const char** glfwExtenstion;

	glfwExtenstion = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

	VkApplicationInfo VKInfo = {};
	VKInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	VKInfo.pApplicationName = "Hello Triangle";
	VKInfo.applicationVersion = VK_MAKE_VERSION(1,0,0);
	VKInfo.pEngineName = "No Engine";
	VKInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
	VKInfo.apiVersion = VK_API_VERSION_1_1;

	auto extensions = GetRequiredExtensions();

	VkInstanceCreateInfo CreateVKInfo = {};
	CreateVKInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	CreateVKInfo.pApplicationInfo = &VKInfo;
	CreateVKInfo.enabledExtensionCount = glfwExtensionCount;
	CreateVKInfo.ppEnabledExtensionNames = glfwExtenstion;
	CreateVKInfo.enabledExtensionCount = static_cast<unsigned int>(extensions.size());
	CreateVKInfo.ppEnabledExtensionNames = extensions.data();

	if (EnableValidationLayers)
	{

		VkDebugUtilsMessengerCreateInfoEXT DebugInfo;
		VulkenDebug.CreateDebugMessengerInfo(DebugInfo);

		CreateVKInfo.enabledLayerCount = static_cast<unsigned int>(ValidationLayers.size());
		CreateVKInfo.ppEnabledLayerNames = ValidationLayers.data();
		CreateVKInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*) &DebugInfo;
	}
	else
	{
		CreateVKInfo.enabledLayerCount = 0;
	}
	
	VkResult result = vkCreateInstance(&CreateVKInfo, nullptr, &VulkanInstance);
	if (result != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create Vulkan instance. Error:" + result);
	}
}

void Vulkan::SetUpDebugger()
{
	if (!EnableValidationLayers)
	{
		return;
	}

	VulkenDebug.SetUpDebugger(VulkanInstance);
}

void Vulkan::SetUpWindowSurface()
{
	VkResult Result = glfwCreateWindowSurface(VulkanInstance, Window.GetWindowPtr(), nullptr, &WindowSurface);
	if (Result != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create window surface.");
	}
}

void Vulkan::SetUpVideoCard()
{
	unsigned int VideoCardCount = 0;
	vkEnumeratePhysicalDevices(VulkanInstance, &VideoCardCount, nullptr);

	if (VideoCardCount == 0)
	{
		throw std::runtime_error("Couldn't find any Video Cards with Vulkan support.");
	}

	std::vector<VkPhysicalDevice> VideoCardList(VideoCardCount);
	vkEnumeratePhysicalDevices(VulkanInstance, &VideoCardCount, VideoCardList.data());

	for (const auto& VideoCard : VideoCardList)
	{
		if (VideoCardVulkanCompatible(VideoCard))
		{
			VideoCardDevice = VideoCard;
			break;
		}
	}

	if (VideoCardDevice == VK_NULL_HANDLE)
	{
		throw std::runtime_error("Couldn't find any Video Cards with Vulkan support.");
	}
}

void Vulkan::SetUpLogicalDevice()
{
	QueueFamilyIndices Indices = QueueFamilies(VideoCardDevice);
	float QueuePriority = 1.0f;

	std::vector<VkDeviceQueueCreateInfo> QueueCreateInfoList;
	std::set<unsigned int> UniqueQueuFamilies = { Indices.GraphicsFamily.value(), Indices.PresentationFamily.value() };

	for (auto QueueFamily : UniqueQueuFamilies)
	{
		VkDeviceQueueCreateInfo QueueCreateInfo = {};
		QueueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		QueueCreateInfo.queueFamilyIndex = Indices.GraphicsFamily.value();
		QueueCreateInfo.queueCount = 1;
		QueueCreateInfo.pQueuePriorities = &QueuePriority;
		QueueCreateInfoList.emplace_back(QueueCreateInfo);
	}

	VkPhysicalDeviceFeatures DeviceFeatures = {};

	VkDeviceCreateInfo CreateInfo = {};
	CreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	CreateInfo.pQueueCreateInfos = QueueCreateInfoList.data();
	CreateInfo.queueCreateInfoCount = static_cast<uint32_t>(QueueCreateInfoList.size());
	CreateInfo.pEnabledFeatures = &DeviceFeatures;
	CreateInfo.enabledExtensionCount = static_cast<unsigned int>(DeviceExtensions.size());
	CreateInfo.ppEnabledExtensionNames = DeviceExtensions.data();

	if (EnableValidationLayers)
	{
		CreateInfo.enabledLayerCount = static_cast<unsigned int>(ValidationLayers.size());
		CreateInfo.ppEnabledLayerNames = ValidationLayers.data();
	}
	else
	{
		CreateInfo.enabledLayerCount = 0;
	}

	VkResult Result = vkCreateDevice(VideoCardDevice, &CreateInfo, nullptr, &Device);
	if (Result != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create logical device.");
	}

	vkGetDeviceQueue(Device, Indices.GraphicsFamily.value(), 0, &GraphicsQueue);
	vkGetDeviceQueue(Device, Indices.PresentationFamily.value(), 0, &PresentationQueue);
}

void Vulkan::SetUpSwapChain()
{
	SwapChainSupportDetails SwapChainSupport = CheckSwapChainSupport(VideoCardDevice);
	VkSurfaceFormatKHR SurfaceFormat = SwapChainFormat(SwapChainSupport.Formats);
	VkPresentModeKHR PresentMode = SwapChainPresentation(SwapChainSupport.PresentModes);
	VkExtent2D Extent = SwapChainExtent(SwapChainSupport.Capabilities);

	unsigned int ImageCount = SwapChainSupport.Capabilities.minImageCount + 1;
	QueueFamilyIndices Indices = QueueFamilies(VideoCardDevice);
	uint32_t queueFamilyIndices[] = { Indices.GraphicsFamily.value(), Indices.PresentationFamily.value() };

	if (SwapChainSupport.Capabilities.maxImageCount > 0 && ImageCount > SwapChainSupport.Capabilities.maxImageCount) 
	{
		ImageCount = SwapChainSupport.Capabilities.maxImageCount;
	}

	VkSwapchainCreateInfoKHR SwapChainInfo = {};
	SwapChainInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	SwapChainInfo.surface = WindowSurface;
	SwapChainInfo.minImageCount = ImageCount;
	SwapChainInfo.imageFormat = SurfaceFormat.format;
	SwapChainInfo.imageColorSpace = SurfaceFormat.colorSpace;
	SwapChainInfo.imageExtent = Extent;
	SwapChainInfo.imageArrayLayers = 1;
	SwapChainInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
	SwapChainInfo.preTransform = SwapChainSupport.Capabilities.currentTransform;
	SwapChainInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	SwapChainInfo.presentMode = PresentMode;
	SwapChainInfo.clipped = VK_TRUE;
	SwapChainInfo.oldSwapchain = VK_NULL_HANDLE;

	if (Indices.GraphicsFamily != Indices.PresentationFamily)
	{
		SwapChainInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
		SwapChainInfo.queueFamilyIndexCount = 2;
		SwapChainInfo.pQueueFamilyIndices = queueFamilyIndices;
	}
	else
	{
		SwapChainInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
		SwapChainInfo.queueFamilyIndexCount = 0;
		SwapChainInfo.pQueueFamilyIndices = nullptr;
	}
	
	VkResult Result = vkCreateSwapchainKHR(Device, &SwapChainInfo, nullptr, &SwapChain);
	if (Result != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create swap chain.");
	}

	vkGetSwapchainImagesKHR(Device, SwapChain, &ImageCount, nullptr);
	SwapChainImages.resize(ImageCount);
	vkGetSwapchainImagesKHR(Device, SwapChain, &ImageCount, SwapChainImages.data());

	SwapChainImageFormat = SurfaceFormat.format;
	SwapChainExtention = Extent;
}

QueueFamilyIndices Vulkan::QueueFamilies(VkPhysicalDevice device)
{
	QueueFamilyIndices Indices;
	unsigned int QueueFamilyCount = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(device, &QueueFamilyCount, nullptr);

	std::vector<VkQueueFamilyProperties> QueueFamilyList(QueueFamilyCount);
	vkGetPhysicalDeviceQueueFamilyProperties(device, &QueueFamilyCount, QueueFamilyList.data());

	for (int x = 0; x <= QueueFamilyList.size() - 1; x++)
	{
		VkBool32 PresentationSupport = false;
		vkGetPhysicalDeviceSurfaceSupportKHR(device, x, WindowSurface, &PresentationSupport);

		if (QueueFamilyList[x].queueFlags && VK_QUEUE_GRAPHICS_BIT)
		{
			Indices.GraphicsFamily = x;
		}

		if (PresentationSupport)
		{
			Indices.PresentationFamily = x;
		}

		if (Indices.IsComplete())
		{
			break;
		}
	}

	return Indices;
}

bool Vulkan::VideoCardVulkanCompatible(VkPhysicalDevice device)
{
	QueueFamilyIndices Indices = QueueFamilies(device);
	bool ExtenstionSupported = VideoCardExtenstionSupport(device);
	bool SwapChainUsable = false;

	if (ExtenstionSupported)
	{
		SwapChainSupportDetails SwapChainSupport = CheckSwapChainSupport(device);
		SwapChainUsable = !SwapChainSupport.Formats.empty() &&
						  !SwapChainSupport.PresentModes.empty();
	}

	VkPhysicalDeviceProperties VideoCardProperties;
	VkPhysicalDeviceFeatures VideoCardFeatures;
	vkGetPhysicalDeviceProperties(device, &VideoCardProperties);
	vkGetPhysicalDeviceFeatures(device, &VideoCardFeatures);

	return VideoCardProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU && 
											 VideoCardFeatures.geometryShader &&
											 ExtenstionSupported &&
											 SwapChainUsable &&
											 Indices.IsComplete();
}

bool Vulkan::VideoCardExtenstionSupport(VkPhysicalDevice device)
{
	unsigned int ExtensionCount;
	vkEnumerateDeviceExtensionProperties(device, nullptr, &ExtensionCount, nullptr);

	std::vector<VkExtensionProperties> AvailableExtensions(ExtensionCount);
	vkEnumerateDeviceExtensionProperties(device, nullptr, &ExtensionCount, AvailableExtensions.data());

	std::set<std::string> RequeriedExtensions(DeviceExtensions.begin(), DeviceExtensions.end());
	for (const auto& Extension : AvailableExtensions)
	{
		RequeriedExtensions.erase(Extension.extensionName);
	}

	return RequeriedExtensions.empty();
}

bool Vulkan::ValidationLayerSupport()
{
	unsigned int LayerCount;
	vkEnumerateInstanceLayerProperties(&LayerCount, nullptr);

	std::vector<VkLayerProperties> AvailableLayers(LayerCount);
	vkEnumerateInstanceLayerProperties(&LayerCount, AvailableLayers.data());

	for (const char* LayerName : ValidationLayers)
	{
		bool LayerFound = false;

		for (const auto& LayerProperties : AvailableLayers)
		{
			if (strcmp(LayerName, LayerProperties.layerName) == 0)
			{
				LayerFound = true;
				break;
			}
		}

		if (!LayerFound)
		{
			return false;
		}
	}

	return true;
}

SwapChainSupportDetails Vulkan::CheckSwapChainSupport(VkPhysicalDevice device)
{
	SwapChainSupportDetails  Details;
	unsigned int FormatCount;
	unsigned int PresentModeCount;

	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, WindowSurface, &Details.Capabilities);
	vkGetPhysicalDeviceSurfaceFormatsKHR(device, WindowSurface, &FormatCount, nullptr);
	vkGetPhysicalDeviceSurfacePresentModesKHR(device, WindowSurface, &PresentModeCount, nullptr);

	if (FormatCount != 0)
	{
		Details.Formats.resize(FormatCount);
		vkGetPhysicalDeviceSurfaceFormatsKHR(device, WindowSurface, &FormatCount, Details.Formats.data());
	}

	if (PresentModeCount != 0)
	{
		Details.PresentModes.resize(PresentModeCount);
		vkGetPhysicalDeviceSurfacePresentModesKHR(device, WindowSurface, &PresentModeCount, Details.PresentModes.data());
	}

	return Details;
}

VkSurfaceFormatKHR Vulkan::SwapChainFormat(const std::vector<VkSurfaceFormatKHR>& AvailableFormatList)
{
	for (const auto& AvailableFormat : AvailableFormatList)
	{
		if (AvailableFormat.format == VK_FORMAT_B8G8R8A8_UNORM &&
			AvailableFormat.format == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
		{
			return AvailableFormat;
		}
	}

	return AvailableFormatList[0];
}

VkPresentModeKHR Vulkan::SwapChainPresentation(const std::vector<VkPresentModeKHR>& AvailablePresentModeList)
{
	for (const auto& AvailablePresentMode : AvailablePresentModeList)
	{
		if (AvailablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR)
		{
			return AvailablePresentMode;
		}
	}

	return VK_PRESENT_MODE_FIFO_KHR;
}

VkExtent2D Vulkan::SwapChainExtent(const VkSurfaceCapabilitiesKHR& capabilities)
{
	if (capabilities.currentExtent.width != UINT32_MAX)
	{
		return capabilities.currentExtent;
	}
	else
	{
		VkExtent2D ActualExtent = { Window.GetWidth(), Window.GetHeight() };
		ActualExtent.width = std::max(capabilities.minImageExtent.width, std::min(capabilities.maxImageExtent.width, ActualExtent.width));
		ActualExtent.height = std::max(capabilities.minImageExtent.width, std::min(capabilities.maxImageExtent.height, ActualExtent.height));
	
		return ActualExtent;
	}
}

std::vector<const char*> Vulkan::GetRequiredExtensions()
{
	unsigned int glfwExtenstionCount = 0;
	const char** glfwExtensions;

	glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtenstionCount);

	std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtenstionCount);
	extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);

	return extensions;
}

void Vulkan::MainLoop()
{
	while (!glfwWindowShouldClose(Window.GetWindowPtr()))
	{
		glfwPollEvents();
	}
}

void Vulkan::Run()
{
	MainLoop();
}

void Vulkan::GetVulkanExtenstions()
{
	unsigned int ExtenstionCount = 0;
	std::vector<VkExtensionProperties> ExtensionList(ExtenstionCount);

	vkEnumerateInstanceExtensionProperties(nullptr, &ExtenstionCount, ExtensionList.data());

	std::cout << "Available Extensions: " << std::endl;
	for (const auto& Extension : ExtensionList)
	{
		std::cout << "\t" << Extension.extensionName << std::endl;
	}
}
