#include "Vulkan.h"

Vulkan::Vulkan(unsigned int width, unsigned int height, const char* windowName) : EnableValidationLayers(true)
{
	VideoCardDevice = VK_NULL_HANDLE;

	Window = VulkanWindow(width, height, windowName);

	ValidationLayers.emplace_back("VK_LAYER_KHRONOS_validation");

	CreateVulkanInstance();
	SetUpDebugger();
	SetUpVideoCard();
}

Vulkan::~Vulkan()
{
	if (EnableValidationLayers)
	{
		VulkenDebug.DestroyDebugUtilsMessengerEXT(VulkanInstance, nullptr);
	}
	vkDestroyInstance(VulkanInstance, nullptr);
	glfwDestroyWindow(Window.GetWindowPtr());
	glfwTerminate();
}

void Vulkan::CreateVulkanInstance()
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

QueueFamilyIndices Vulkan::QueueFamilies(VkPhysicalDevice device)
{
	QueueFamilyIndices Indices;
	unsigned int QueueFamilyCount = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(device, &QueueFamilyCount, nullptr);

	std::vector<VkQueueFamilyProperties> QueueFamilyList(QueueFamilyCount);
	vkGetPhysicalDeviceQueueFamilyProperties(device, &QueueFamilyCount, QueueFamilyList.data());

	for (int x = 0; x <= QueueFamilyList.size() - 1; x++)
	{
		if (QueueFamilyList[x].queueFlags && VK_QUEUE_GRAPHICS_BIT)
		{
			Indices.GraphicsFamily = x;
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

	VkPhysicalDeviceProperties VideoCardProperties;
	VkPhysicalDeviceFeatures VideoCardFeatures;
	vkGetPhysicalDeviceProperties(device, &VideoCardProperties);
	vkGetPhysicalDeviceFeatures(device, &VideoCardFeatures);

	return VideoCardProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU && 
											 VideoCardFeatures.geometryShader &&
											 Indices.IsComplete();
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

	std::cout << "Availabe Extensions: " << std::endl;
	for (const auto& Extension : ExtensionList)
	{
		std::cout << "\t" << Extension.extensionName << std::endl;
	}
}
