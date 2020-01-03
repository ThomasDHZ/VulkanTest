#include "VulkanWindow.h"

VulkanWindow::VulkanWindow()
{
	Width = 0;
	Height = 0;
	Window = nullptr;
}

VulkanWindow::VulkanWindow(unsigned int width, unsigned int height, const char* windowName)
{
	Width = width;
	Height = height;

	glfwInit();

	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

	Window = glfwCreateWindow(width, height, windowName, nullptr, nullptr);
	glfwSetWindowUserPointer(Window, this);
	glfwSetFramebufferSizeCallback(Window, framebufferResizeCallback);
}

VulkanWindow::~VulkanWindow()
{
	
}

VulkanWindow& VulkanWindow::operator=(const VulkanWindow& rhs)
{
	Width = rhs.Width;
	Height = rhs.Height;
	Window = rhs.Window;

	return *this;
}
