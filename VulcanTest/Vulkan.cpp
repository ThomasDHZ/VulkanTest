#include "Vulkan.h"

VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger) {
	
	auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
	
	if (func != nullptr) 
	{
		return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
	}
	else 
	{
		return VK_ERROR_EXTENSION_NOT_PRESENT;
	}
}

void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT deubugMessenger, const VkAllocationCallbacks* pAllocator)
{
	auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");

	if (func != nullptr)
	{
		func(instance, deubugMessenger, pAllocator);
	}
}

Vulkan::Vulkan(unsigned int width, unsigned int height, const char* windowName) : EnableValidationLayers(true)
{
	ValidationLayers.emplace_back("VK_LAYER_KHRONOS_validation"); 

	Window = VulkanWindow(width, height, windowName);

	CreateVulkanInstance();
	SetUpDebugger();
}

Vulkan::~Vulkan()
{
	if (EnableValidationLayers)
	{
		DestroyDebugUtilsMessengerEXT(VulkanInstance, debugMessenger, nullptr);
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
		CreateDebugMessengerInfo(DebugInfo);

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

void Vulkan::CreateDebugMessengerInfo(VkDebugUtilsMessengerCreateInfoEXT& DebugInfo)
{
	DebugInfo = {};
	DebugInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
	DebugInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
	DebugInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
	DebugInfo.pfnUserCallback = DebugCallBack;
}

void Vulkan::SetUpDebugger()
{
	if (!EnableValidationLayers)
	{
		return;
	}

	VkDebugUtilsMessengerCreateInfoEXT DebugInfo;
	CreateDebugMessengerInfo(DebugInfo);

	if (CreateDebugUtilsMessengerEXT(VulkanInstance, &DebugInfo, nullptr, &debugMessenger) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to set up debug messenger!");
	}
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

	if (EnableValidationLayers)
	{
		extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
	}

	return extensions;
}

VKAPI_ATTR VkBool32 VKAPI_CALL Vulkan::DebugCallBack(VkDebugUtilsMessageSeverityFlagBitsEXT MessageSeverity, VkDebugUtilsMessageTypeFlagsEXT MessageType, const VkDebugUtilsMessengerCallbackDataEXT* CallBackData, void* UserData)
{
	std::cerr << "Validation Layer: " << CallBackData->pMessage << std::endl;
	return VK_FALSE;
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
