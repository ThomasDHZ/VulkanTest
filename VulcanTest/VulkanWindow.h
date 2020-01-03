#pragma once
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

class VulkanWindow
{
private:
	unsigned int Width;
	unsigned int Height;
	bool framebufferResized = false;

	GLFWwindow* Window;

	static void framebufferResizeCallback(GLFWwindow* window, int width, int height) 
	{
		auto app = reinterpret_cast<VulkanWindow*>(glfwGetWindowUserPointer(window));
		app->framebufferResized = true;
	}

public:
	VulkanWindow();
	VulkanWindow(unsigned int width, unsigned int height, const char* windowName);
	~VulkanWindow();

	void SetFrameBufferResized(bool flag) { framebufferResized = flag; }

	GLFWwindow* GetWindowPtr() { return Window; }
	unsigned int GetWidth() { return Width; }
	unsigned int GetHeight() { return Height; }
	bool GetFramebufferResized() { return framebufferResized; }
	VulkanWindow& operator=(const VulkanWindow& rhs);
};

