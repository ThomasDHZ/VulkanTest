#include "Vulkan.h"

Vulkan::Vulkan(unsigned int width, unsigned int height, const char* windowName) : EnableValidationLayers(true)
{
	Window = VulkanWindow(width, height, windowName);

	ValidationLayers.emplace_back("VK_LAYER_KHRONOS_validation");

	CreateVulkanInstance();
	SetUpDebugger();
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
