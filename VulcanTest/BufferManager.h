#include <vulkan\vulkan_core.h>

static class BufferManager
{
private:
	static uint32_t findMemoryType(VkPhysicalDevice physicalDevice, uint32_t typeFilter, VkMemoryPropertyFlags properties);
public:
	static VkCommandBuffer StartCommandBuffer(VkDevice Device, VkCommandPool CommandPool);
	static void EndCommandBuffer(VkDevice Device, VkQueue GraphicsQueue, VkCommandPool CommandPool, VkCommandBuffer CommandBuffer);
	static void CreateBuffer(VkPhysicalDevice physicalDevice, VkDevice device, VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory);
	static void CopyBuffer(VkDevice device, VkCommandPool commandPool, VkQueue graphicsQueue, VkPhysicalDevice physicalDevice, VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);
};
