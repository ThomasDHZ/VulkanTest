#include <iostream>
#include "Vulkan.h"

int main() 
{
	Vulkan vulkan = Vulkan(3840, 2160, "VulkanTesting");

	try
	{
		vulkan.Run();
	}
	catch (const std::exception & ex) 
	{
		std::cerr << ex.what() << std::endl;
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}