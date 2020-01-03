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
	SetUpImageView();
	SetUpRenderPass();
	SetUpDescriptorSetLayout();
	SetUpGraphicsPipline();
	SetUpFrameBuffer();
	SetUpCommandPool();
	SetUpVertexBuffer();
	SetUpIndexBuffer();
	SetUpCommandBuffers();
	SetUpSyncObjects();
}

Vulkan::~Vulkan()
{
	CleanUpSwapChain();

	vkDestroyBuffer(Device, indexBuffer, nullptr);
	vkFreeMemory(Device, indexBufferMemory, nullptr);

	vkDestroyBuffer(Device, VertexBuffer, nullptr);
	vkFreeMemory(Device, VertexBufferMemory, nullptr);

	for (size_t x = 0; x < MAX_FRAMES_IN_FLIGHT; x++)
	{
		vkDestroySemaphore(Device, RenderFinishedSemaphore[x], nullptr);
		vkDestroySemaphore(Device, ImageAvailableSemaphore[x], nullptr);
		vkDestroyFence(Device, inFlightFences[x], nullptr);
	}

	vkDestroyCommandPool(Device, CommandPool, nullptr);

	for (auto Framebuffer : SwapChainFramebuffers) 
	{
		vkDestroyFramebuffer(Device, Framebuffer, nullptr);
	}

	vkDestroyPipeline(Device, GraphicsPipeline, nullptr);
	vkDestroyPipelineLayout(Device, PipelineLayout, nullptr);
	vkDestroyRenderPass(Device, RenderPass, nullptr);

	for (auto ImageView : SwapChainImageViews)
	{
		vkDestroyImageView(Device, ImageView, nullptr);
	}

	vkDestroySwapchainKHR(Device, SwapChain, nullptr);
	vkDestroyDevice(Device, nullptr);

	if (EnableValidationLayers)
	{
		VulkenDebug.DestroyDebugUtilsMessengerEXT(VulkanInstance, nullptr);
	}

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

void Vulkan::SetUpImageView()
{
	SwapChainImageViews.resize(SwapChainImages.size());

	for (int x = 0; x < SwapChainImages.size(); x++)
	{
		VkImageViewCreateInfo ImageViewInfo{};
		ImageViewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		ImageViewInfo.image = SwapChainImages[x];
		ImageViewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		ImageViewInfo.format = SwapChainImageFormat;
		ImageViewInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
		ImageViewInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
		ImageViewInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
		ImageViewInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
		ImageViewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		ImageViewInfo.subresourceRange.baseMipLevel = 0;
		ImageViewInfo.subresourceRange.levelCount = 1;
		ImageViewInfo.subresourceRange.baseArrayLayer = 0;
		ImageViewInfo.subresourceRange.layerCount = 1;

		VkResult Result = vkCreateImageView(Device, &ImageViewInfo, nullptr, &SwapChainImageViews[x]);
		if (Result != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to create image view.");
		}
	}
}

void Vulkan::SetUpRenderPass()
{
	VkAttachmentDescription ColorAttachment = {};
	ColorAttachment.format = SwapChainImageFormat;
	ColorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
	ColorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	ColorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	ColorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	ColorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	ColorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	ColorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

	VkAttachmentReference ColorAttachmentRef = {};
	ColorAttachmentRef.attachment = 0;
	ColorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
	
	VkSubpassDescription Subpass = {};
	Subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	Subpass.colorAttachmentCount = 1;
	Subpass.pColorAttachments = &ColorAttachmentRef;

	VkSubpassDependency SubpassDependency = {};
	SubpassDependency.srcSubpass = VK_SUBPASS_EXTERNAL;
	SubpassDependency.dstSubpass = 0;
	SubpassDependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	SubpassDependency.srcAccessMask = 0;
	SubpassDependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	SubpassDependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

	VkRenderPassCreateInfo RenderPassInfo = {};
	RenderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	RenderPassInfo.attachmentCount = 1;
	RenderPassInfo.pAttachments = &ColorAttachment;
	RenderPassInfo.subpassCount = 1;
	RenderPassInfo.pSubpasses = &Subpass;
	RenderPassInfo.dependencyCount = 1;
	RenderPassInfo.pDependencies = &SubpassDependency;

	VkResult Result = vkCreateRenderPass(Device, &RenderPassInfo, nullptr, &RenderPass);
	if (Result != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create render pass.");
	}
}

void Vulkan::SetUpDescriptorSetLayout()
{
}

void Vulkan::SetUpGraphicsPipline()
{
	VkShaderModule VertexShaderModule = SCompiler.CompileShader(Device, "vert.spv");
	VkShaderModule FragmentShaderModule = SCompiler.CompileShader(Device, "frag.spv");

	VkPipelineShaderStageCreateInfo VertexShaderStageInfo = {};
	VertexShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	VertexShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
	VertexShaderStageInfo.module = VertexShaderModule;
	VertexShaderStageInfo.pName = "main";

	VkPipelineShaderStageCreateInfo FragmentShaderStageInfo = {};
	FragmentShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	FragmentShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
	FragmentShaderStageInfo.module = FragmentShaderModule;
	FragmentShaderStageInfo.pName = "main";

	VkPipelineShaderStageCreateInfo ShaderStages[] = { VertexShaderStageInfo, FragmentShaderStageInfo };

	auto bindingDescription = Vertex::GetBindingDescription();
	auto attributeDescriptions = Vertex::GetAttributeDescriptions();

	VkPipelineVertexInputStateCreateInfo VertexInputInfo = {};
	VertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	VertexInputInfo.vertexBindingDescriptionCount = 1;
	VertexInputInfo.vertexAttributeDescriptionCount = static_cast<unsigned int>(attributeDescriptions.size());
	VertexInputInfo.pVertexBindingDescriptions = &bindingDescription;
	VertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();

	VkPipelineInputAssemblyStateCreateInfo InputAssembly = {};
	InputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	InputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	InputAssembly.primitiveRestartEnable = VK_FALSE;

	VkViewport Viewport = {};
	Viewport.x = 0.0f;
	Viewport.y = 0.0f;
	Viewport.width = (float)SwapChainExtention.width;
	Viewport.height = (float)SwapChainExtention.height;
	Viewport.minDepth = 0.0f;
	Viewport.maxDepth = 1.0f;

	VkRect2D Scissor = {};
	Scissor.offset = { 0, 0 };
	Scissor.extent = SwapChainExtention;

	VkPipelineViewportStateCreateInfo ViewportState = {};
	ViewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	ViewportState.viewportCount = 1;
	ViewportState.pViewports = &Viewport;
	ViewportState.scissorCount = 1;
	ViewportState.pScissors = &Scissor;

	VkPipelineRasterizationStateCreateInfo Rasterizer = {};
	Rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	Rasterizer.depthClampEnable = VK_FALSE;
	Rasterizer.rasterizerDiscardEnable = VK_FALSE;
	Rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
	Rasterizer.lineWidth = 1.0f;
	Rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
	Rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;
	Rasterizer.depthBiasEnable = VK_FALSE;

	VkPipelineMultisampleStateCreateInfo Multisampling = {};
	Multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	Multisampling.sampleShadingEnable = VK_FALSE;
	Multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

	VkPipelineColorBlendAttachmentState ColorBlendAttachment = {};
	ColorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
	ColorBlendAttachment.blendEnable = VK_FALSE;

	VkPipelineColorBlendStateCreateInfo ColorBlending = {};
	ColorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	ColorBlending.logicOpEnable = VK_FALSE;
	ColorBlending.logicOp = VK_LOGIC_OP_COPY;
	ColorBlending.attachmentCount = 1;
	ColorBlending.pAttachments = &ColorBlendAttachment;
	ColorBlending.blendConstants[0] = 0.0f;
	ColorBlending.blendConstants[1] = 0.0f;
	ColorBlending.blendConstants[2] = 0.0f;
	ColorBlending.blendConstants[3] = 0.0f;

	VkPipelineLayoutCreateInfo PipelineLayoutInfo = {};
	PipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	PipelineLayoutInfo.setLayoutCount = 0;
	PipelineLayoutInfo.pushConstantRangeCount = 0;

	if (vkCreatePipelineLayout(Device, &PipelineLayoutInfo, nullptr, &PipelineLayout) != VK_SUCCESS) {
		throw std::runtime_error("Failed to create pipeline layout.");
	}

	VkGraphicsPipelineCreateInfo PipelineInfo = {};
	PipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	PipelineInfo.stageCount = 2;
	PipelineInfo.pStages = ShaderStages;
	PipelineInfo.pVertexInputState = &VertexInputInfo;
	PipelineInfo.pInputAssemblyState = &InputAssembly;
	PipelineInfo.pViewportState = &ViewportState;
	PipelineInfo.pRasterizationState = &Rasterizer;
	PipelineInfo.pMultisampleState = &Multisampling;
	PipelineInfo.pColorBlendState = &ColorBlending;
	PipelineInfo.layout = PipelineLayout;
	PipelineInfo.renderPass = RenderPass;
	PipelineInfo.subpass = 0;
	PipelineInfo.basePipelineHandle = VK_NULL_HANDLE;

	VkResult CreateGraphicsPipelineResult = vkCreateGraphicsPipelines(Device, VK_NULL_HANDLE, 1, &PipelineInfo, nullptr, &GraphicsPipeline);
	if (CreateGraphicsPipelineResult != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create graphics pipeline.");
	}

	vkDestroyShaderModule(Device, FragmentShaderModule, nullptr);
	vkDestroyShaderModule(Device, VertexShaderModule, nullptr);
}

void Vulkan::SetUpFrameBuffer()
{
	SwapChainFramebuffers.resize(SwapChainImageViews.size());
	for (size_t x = 0; x < SwapChainImageViews.size(); x++)
	{
		VkImageView Attachment[] =
		{
			SwapChainImageViews[x]
		};

		VkFramebufferCreateInfo FrameBufferInfo = {};
		FrameBufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		FrameBufferInfo.renderPass = RenderPass;
		FrameBufferInfo.attachmentCount = 1;
		FrameBufferInfo.pAttachments = Attachment;
		FrameBufferInfo.width = SwapChainExtention.width;
		FrameBufferInfo.height = SwapChainExtention.height;
		FrameBufferInfo.layers = 1;

		VkResult Result = vkCreateFramebuffer(Device, &FrameBufferInfo, nullptr, &SwapChainFramebuffers[x]);
		if (Result != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to create framebuffer.");
		}
	}

}

void Vulkan::SetUpCommandPool()
{
	QueueFamilyIndices QueueFamilyIndices = QueueFamilies(VideoCardDevice);

	VkCommandPoolCreateInfo PoolInfo = {};
	PoolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	PoolInfo.queueFamilyIndex = QueueFamilyIndices.GraphicsFamily.value();

	VkResult Result = vkCreateCommandPool(Device, &PoolInfo, nullptr, &CommandPool);
	if (Result != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create framebuffer.");
	}
}

void Vulkan::SetUpVertexBuffer()
{
	VkDeviceSize bufferSize = sizeof(vertices[0]) * vertices.size();

	VkBuffer stagingBuffer;
	VkDeviceMemory stagingBufferMemory;
	CreateBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

	void* data;
	vkMapMemory(Device, stagingBufferMemory, 0, bufferSize, 0, &data);
	memcpy(data, vertices.data(), (size_t)bufferSize);
	vkUnmapMemory(Device, stagingBufferMemory);

	CreateBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, VertexBuffer, VertexBufferMemory);

	CopyBuffer(stagingBuffer, VertexBuffer, bufferSize);

	vkDestroyBuffer(Device, stagingBuffer, nullptr);
	vkFreeMemory(Device, stagingBufferMemory, nullptr);
}

void Vulkan::SetUpIndexBuffer()
{
	VkDeviceSize bufferSize = sizeof(indices[0]) * indices.size();

	VkBuffer stagingBuffer;
	VkDeviceMemory stagingBufferMemory;
	CreateBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

	void* data;
	vkMapMemory(Device, stagingBufferMemory, 0, bufferSize, 0, &data);
	memcpy(data, indices.data(), (size_t)bufferSize);
	vkUnmapMemory(Device, stagingBufferMemory);

	CreateBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, indexBuffer, indexBufferMemory);

	CopyBuffer(stagingBuffer, indexBuffer, bufferSize);

	vkDestroyBuffer(Device, stagingBuffer, nullptr);
	vkFreeMemory(Device, stagingBufferMemory, nullptr);
}

void Vulkan::SetUpCommandBuffers()
{
	CommandBuffers.resize(SwapChainFramebuffers.size());

	VkCommandBufferAllocateInfo allocInfo = {};
	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.commandPool = CommandPool;
	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocInfo.commandBufferCount = (uint32_t)CommandBuffers.size();

	if (vkAllocateCommandBuffers(Device, &allocInfo, CommandBuffers.data()) != VK_SUCCESS) {
		throw std::runtime_error("failed to allocate command buffers!");
	}

	for (size_t i = 0; i < CommandBuffers.size(); i++) {
		VkCommandBufferBeginInfo beginInfo = {};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

		if (vkBeginCommandBuffer(CommandBuffers[i], &beginInfo) != VK_SUCCESS) {
			throw std::runtime_error("failed to begin recording command buffer!");
		}

		VkRenderPassBeginInfo renderPassInfo = {};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderPassInfo.renderPass = RenderPass;
		renderPassInfo.framebuffer = SwapChainFramebuffers[i];
		renderPassInfo.renderArea.offset = { 0, 0 };
		renderPassInfo.renderArea.extent = SwapChainExtention;

		VkClearValue clearColor = { 0.0f, 0.0f, 0.0f, 1.0f };
		renderPassInfo.clearValueCount = 1;
		renderPassInfo.pClearValues = &clearColor;

		vkCmdBeginRenderPass(CommandBuffers[i], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

		vkCmdBindPipeline(CommandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, GraphicsPipeline);

		VkBuffer vertexBuffers[] = { VertexBuffer };
		VkDeviceSize offsets[] = { 0 };
		vkCmdBindVertexBuffers(CommandBuffers[i], 0, 1, vertexBuffers, offsets);

		vkCmdBindIndexBuffer(CommandBuffers[i], indexBuffer, 0, VK_INDEX_TYPE_UINT16);

		vkCmdDrawIndexed(CommandBuffers[i], static_cast<uint32_t>(indices.size()), 1, 0, 0, 0);

		vkCmdEndRenderPass(CommandBuffers[i]);

		if (vkEndCommandBuffer(CommandBuffers[i]) != VK_SUCCESS) {
			throw std::runtime_error("failed to record command buffer!");
		}
	}
}

void Vulkan::SetUpSyncObjects()
{
	ImageAvailableSemaphore.resize(MAX_FRAMES_IN_FLIGHT);
	RenderFinishedSemaphore.resize(MAX_FRAMES_IN_FLIGHT);
	inFlightFences.resize(MAX_FRAMES_IN_FLIGHT);
	imagesInFlight.resize(SwapChainImages.size(), VK_NULL_HANDLE);

	VkSemaphoreCreateInfo SemaphoreInfo = {};
	SemaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

	VkFenceCreateInfo FenceInfo = {};
	FenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	FenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

	for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) 
	{
		if (vkCreateSemaphore(Device, &SemaphoreInfo, nullptr, &ImageAvailableSemaphore[i]) != VK_SUCCESS ||
			vkCreateSemaphore(Device, &SemaphoreInfo, nullptr, &RenderFinishedSemaphore[i]) != VK_SUCCESS ||
			vkCreateFence(Device, &FenceInfo, nullptr, &inFlightFences[i]) != VK_SUCCESS) {

			throw std::runtime_error("failed to create synchronization objects for a frame!");
		}
	}
}

void Vulkan::RecreateSwapChain()
{
	int width = 0, height = 0;
	glfwGetFramebufferSize(Window.GetWindowPtr(), &width, &height);
	while (width == 0 || height == 0) {
		glfwGetFramebufferSize(Window.GetWindowPtr(), &width, &height);
		glfwWaitEvents();
	}

	vkDeviceWaitIdle(Device);

	CleanUpSwapChain();

	SetUpSwapChain();
	SetUpImageView();
	SetUpRenderPass();
	SetUpGraphicsPipline();
	SetUpFrameBuffer();
	SetUpCommandBuffers();
}

void Vulkan::CleanUpSwapChain()
{
	for (size_t i = 0; i < SwapChainFramebuffers.size(); i++) {
		vkDestroyFramebuffer(Device, SwapChainFramebuffers[i], nullptr);
	}

	vkFreeCommandBuffers(Device, CommandPool, static_cast<uint32_t>(CommandBuffers.size()), CommandBuffers.data());

	vkDestroyPipeline(Device, GraphicsPipeline, nullptr);
	vkDestroyPipelineLayout(Device, PipelineLayout, nullptr);
	vkDestroyRenderPass(Device, RenderPass, nullptr);

	for (size_t i = 0; i < SwapChainImageViews.size(); i++) {
		vkDestroyImageView(Device, SwapChainImageViews[i], nullptr);
	}

	vkDestroySwapchainKHR(Device, SwapChain, nullptr);
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
		int width, height;
		glfwGetFramebufferSize(Window.GetWindowPtr(), &width, &height);

		VkExtent2D ActualExtent = {
			static_cast<uint32_t>(width),
			static_cast<uint32_t>(height)
		};
		ActualExtent.width = std::max(capabilities.minImageExtent.width, std::min(capabilities.maxImageExtent.width, ActualExtent.width));
		ActualExtent.height = std::max(capabilities.minImageExtent.width, std::min(capabilities.maxImageExtent.height, ActualExtent.height));
	
		return ActualExtent;
	}
}

uint32_t Vulkan::findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties)
{
	VkPhysicalDeviceMemoryProperties memProperties;
	vkGetPhysicalDeviceMemoryProperties(VideoCardDevice, &memProperties);

	for (unsigned int x = 0; x < memProperties.memoryTypeCount; x++)
	{
		if ((typeFilter & (1 << x)) && (memProperties.memoryTypes[x].propertyFlags & properties) == properties) 
		{
			return x;
		}
	}

	throw std::runtime_error("failed to find suitable memory type!");
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
		Draw();
	}
	vkDeviceWaitIdle(Device);
}

void Vulkan::CreateBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory)
{
	VkBufferCreateInfo bufferInfo = {};
	bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	bufferInfo.size = size;
	bufferInfo.usage = usage;
	bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

	if (vkCreateBuffer(Device, &bufferInfo, nullptr, &buffer) != VK_SUCCESS) {
		throw std::runtime_error("failed to create buffer!");
	}

	VkMemoryRequirements memRequirements;
	vkGetBufferMemoryRequirements(Device, buffer, &memRequirements);

	VkMemoryAllocateInfo allocInfo = {};
	allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocInfo.allocationSize = memRequirements.size;
	allocInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, properties);

	if (vkAllocateMemory(Device, &allocInfo, nullptr, &bufferMemory) != VK_SUCCESS) {
		throw std::runtime_error("failed to allocate buffer memory!");
	}

	vkBindBufferMemory(Device, buffer, bufferMemory, 0);
}

void Vulkan::CopyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size)
{
	VkCommandBufferAllocateInfo AllocationInfo = {};
	AllocationInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	AllocationInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	AllocationInfo.commandPool = CommandPool;
	AllocationInfo.commandBufferCount = 1;

	VkCommandBuffer CommandBuffer;
	vkAllocateCommandBuffers(Device, &AllocationInfo, &CommandBuffer);

	VkCommandBufferBeginInfo BeginInfo = {};
	BeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	BeginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

	vkBeginCommandBuffer(CommandBuffer, &BeginInfo);

	VkBufferCopy CopyRegion = {};
	CopyRegion.size = size;
	vkCmdCopyBuffer(CommandBuffer, srcBuffer, dstBuffer, 1, &CopyRegion);

	vkEndCommandBuffer(CommandBuffer);

	VkSubmitInfo SubmitInfo = {};
	SubmitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	SubmitInfo.commandBufferCount = 1;
	SubmitInfo.pCommandBuffers = &CommandBuffer;

	vkQueueSubmit(GraphicsQueue, 1, &SubmitInfo, VK_NULL_HANDLE);
	vkQueueWaitIdle(GraphicsQueue);

	vkFreeCommandBuffers(Device, CommandPool, 1, &CommandBuffer);
}

void Vulkan::Run()
{
	MainLoop();
}

void Vulkan::Draw()
{
	vkWaitForFences(Device, 1, &inFlightFences[CurrentFrame], VK_TRUE, UINT64_MAX);

	uint32_t imageIndex;
	VkResult result = vkAcquireNextImageKHR(Device, SwapChain, UINT64_MAX, ImageAvailableSemaphore[CurrentFrame], VK_NULL_HANDLE, &imageIndex);

	if (result == VK_ERROR_OUT_OF_DATE_KHR) {
		RecreateSwapChain();
		return;
	}
	else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
		throw std::runtime_error("failed to acquire swap chain image!");
	}

	if (imagesInFlight[imageIndex] != VK_NULL_HANDLE) {
		vkWaitForFences(Device, 1, &imagesInFlight[imageIndex], VK_TRUE, UINT64_MAX);
	}
	imagesInFlight[imageIndex] = inFlightFences[CurrentFrame];

	VkSubmitInfo submitInfo = {};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

	VkSemaphore waitSemaphores[] = { ImageAvailableSemaphore[CurrentFrame] };
	VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
	submitInfo.waitSemaphoreCount = 1;
	submitInfo.pWaitSemaphores = waitSemaphores;
	submitInfo.pWaitDstStageMask = waitStages;

	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &CommandBuffers[imageIndex];

	VkSemaphore signalSemaphores[] = { RenderFinishedSemaphore[CurrentFrame] };
	submitInfo.signalSemaphoreCount = 1;
	submitInfo.pSignalSemaphores = signalSemaphores;

	vkResetFences(Device, 1, &inFlightFences[CurrentFrame]);

	if (vkQueueSubmit(GraphicsQueue, 1, &submitInfo, inFlightFences[CurrentFrame]) != VK_SUCCESS) {
		throw std::runtime_error("failed to submit draw command buffer!");
	}

	VkPresentInfoKHR presentInfo = {};
	presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

	presentInfo.waitSemaphoreCount = 1;
	presentInfo.pWaitSemaphores = signalSemaphores;

	VkSwapchainKHR swapChains[] = { SwapChain };
	presentInfo.swapchainCount = 1;
	presentInfo.pSwapchains = swapChains;

	presentInfo.pImageIndices = &imageIndex;

	result = vkQueuePresentKHR(PresentationQueue, &presentInfo);

	if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || Window.GetFramebufferResized()) 
	{
		Window.SetFrameBufferResized(false);
		RecreateSwapChain();
	}
	else if (result != VK_SUCCESS) {
		throw std::runtime_error("failed to present swap chain image!");
	}

	CurrentFrame = (CurrentFrame + 1) % MAX_FRAMES_IN_FLIGHT;

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
