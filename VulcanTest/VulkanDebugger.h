#pragma once
#include <vulkan/vulkan.h>

#include <iostream>
#include <stdexcept>
#include <functional>
#include <cstdlib>

class VulkanDebugger
{
private:

	VkDebugUtilsMessengerEXT DebugMessenger;

	VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger);

	static VKAPI_ATTR VkBool32 VKAPI_CALL DebugCallBack(VkDebugUtilsMessageSeverityFlagBitsEXT MessageSeverity, VkDebugUtilsMessageTypeFlagsEXT MessageType, const VkDebugUtilsMessengerCallbackDataEXT* CallBackData, void* UserData);
public:
	VulkanDebugger();


	void SetUpDebugger(VkInstance& VulkanInstance);
	void CreateDebugMessengerInfo(VkDebugUtilsMessengerCreateInfoEXT& DebugInfo);
	void DestroyDebugUtilsMessengerEXT(VkInstance instance, const VkAllocationCallbacks* pAllocator);

	VkDebugUtilsMessengerEXT GetDebugMessenger() { return DebugMessenger; }
};

