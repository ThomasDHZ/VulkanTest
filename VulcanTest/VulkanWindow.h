#pragma once
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

class VulkanWindow
{
private:
	unsigned int Width;
	unsigned int Height;

	GLFWwindow* Window;

public:
	VulkanWindow();
	VulkanWindow(unsigned int width, unsigned int height, const char* windowName);
	~VulkanWindow();

	GLFWwindow* GetWindowPtr() { return Window; }

	VulkanWindow& operator=(const VulkanWindow& rhs);
};

