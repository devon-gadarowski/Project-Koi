
#include <RenderFramework.h>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include <dds.h>

#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader.h>

#include <fstream>
#include <unordered_map>

namespace RenderFramework
{
void createBorderlessWindow(GLFWwindow ** window )
{
	glfwInit();

	GLFWmonitor * monitor = glfwGetPrimaryMonitor();

	const GLFWvidmode * mode = glfwGetVideoMode(monitor);

	glfwWindowHint(GLFW_RED_BITS, mode->redBits);
	glfwWindowHint(GLFW_GREEN_BITS, mode->greenBits);
	glfwWindowHint(GLFW_BLUE_BITS, mode->blueBits);
	glfwWindowHint(GLFW_REFRESH_RATE, mode->refreshRate);
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

	glfwWindowHint(GLFW_AUTO_ICONIFY, GL_FALSE);

	*window = glfwCreateWindow(mode->width, mode->height, "Project Koi", monitor, NULL);

	DEBUG("RENDER_FRAMEWORK - Window Created");
}

void destroyWindow(GLFWwindow ** window)
{
	glfwDestroyWindow(*window);
	glfwTerminate();

	DEBUG("RENDER_FRAMEWORK - Window Destroyed");
}

void createVkSurfaceKHR(VkInstance instance, GLFWwindow * window, VkSurfaceKHR * surface)
{
	if (window == nullptr)
		return;

	int result = glfwCreateWindowSurface(instance, window, nullptr, surface);
	if (result != VK_SUCCESS)
	{
		PANIC("RENDER_FRAMEWORK - Failed to create a window surface! %d", result);
	}
}

void destroyVkSurfaceKHR(VkInstance instance, VkSurfaceKHR * surface)
{
	vkDestroySurfaceKHR(instance, *surface, nullptr);
}

// Returns a handle to a VkInstance object with default parameters
void createInstance(VkInstance * instance)
{
	createInstance(nullptr, 0, nullptr, 0, instance);
}

// Returns a handle to a VkInstance object with GLFW window support
void createInstanceGLFW(std::vector<const char *> layers, std::vector<const char *> extensions, VkInstance * instance)
{
	uint32_t instanceExtensionCount = 0;
	const char ** instanceExtensions = nullptr;

	instanceExtensions = glfwGetRequiredInstanceExtensions(&instanceExtensionCount);

	for (int i = 0; i < instanceExtensionCount; i++)
		extensions.push_back(instanceExtensions[i]);

	createInstance(layers, extensions, instance);
}

// Returns a handle to a VkInstance object
void createInstance(std::vector<const char *> layers, std::vector<const char *> extensions, VkInstance * instance)
{
	createInstance(layers.data(), layers.size(), extensions.data(), extensions.size(), instance);
}

// Returns a handle to a VkInstance object
void createInstance(const char ** layers, uint32_t layerCount, const char ** extensions, uint32_t extensionCount, VkInstance * instance)
{
	VkApplicationInfo appInfo = {};
	appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	appInfo.pNext = nullptr;
	appInfo.pApplicationName = APPLICATION_NAME;
	appInfo.applicationVersion = APPLICATION_VERSION;
	appInfo.pEngineName = ENGINE_NAME;
	appInfo.engineVersion = ENGINE_VERSION;
	appInfo.apiVersion = VK_API_VERSION_1_0;

	VkInstanceCreateInfo createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	createInfo.flags = 0;
	createInfo.pApplicationInfo = &appInfo;
	createInfo.enabledLayerCount = validationLayers.size();
	createInfo.ppEnabledLayerNames = validationLayers.data();
	createInfo.enabledExtensionCount = extensionCount;
	createInfo.ppEnabledExtensionNames = extensions;

	int result = vkCreateInstance(&createInfo, nullptr, instance);
	if (result != VK_SUCCESS)
	{
		PANIC("RENDER_FRAMEWORK - Failed to create VkInstance! error_code: %d", result);
		return;
	}

	DEBUG("RENDER_FRAMEWORK - VkInstance Created");
}

// Destroys the VkInstance object instance and frees associated resources
void destroyInstance(VkInstance * instance)
{
	vkDestroyInstance(*instance, nullptr);

	DEBUG("RENDER_FRAMEWORK - VkInstance Destroyed");
}

// Returns a list of all Graphics Hardware with Vulkan support 
std::vector<VkPhysicalDevice> enumeratePhysicalDevices(VkInstance instance)
{
	uint32_t deviceCount = 0;
	vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);
	std::vector<VkPhysicalDevice> devices(deviceCount);
	vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data());

	return devices;
}

void chooseVkPhysicalDevice(VkInstance instance, VkPhysicalDevice * physicalDevice)
{
	std::vector<VkPhysicalDevice> devices = enumeratePhysicalDevices(instance);

	// Check each available device against the required characteristics
	for (auto& device : devices)
	{
		std::vector<VkQueueFamilyProperties> queueProperties = getDeviceQueueProperties(device);
		std::vector<VkExtensionProperties> extensionProperties = getDeviceExtensions(device);

		// Verify all required queues are available
		bool suitable = true;
		for (auto& queueFamily : requiredQueues)
		{
			bool found = false;
			for (auto& property : queueProperties)
			{
				if (queueFamily & property.queueFlags)
				{
					found = true;
					break;
				}
			}

			if (!found)
			{
				suitable = false;
			}
		}

		if (!suitable)
		{
			continue;
		}

		// Verify all required extensions are available
		for (auto& extensionName : deviceExtensions)
		{
			bool found = false;
			for (auto& property : extensionProperties)
			{
				if (strcmp(property.extensionName, extensionName))
				{
					found = true;
					break;
				}
			}

			if (!found)
			{
				suitable = false;
			}
		}

		// Verify all required features are available
		VkPhysicalDeviceFeatures supportedFeatures;
		vkGetPhysicalDeviceFeatures(device, &supportedFeatures);

		if (!supportedFeatures.samplerAnisotropy)
		{
			suitable = false;
		}

		if (!suitable)
		{
			continue;
		}

		*physicalDevice = device;
		break;
	}

	if (*physicalDevice == VK_NULL_HANDLE)
	{
		PANIC("RENDER_FRAMEWORK - Failed to find suitable VkPhysicalDevice!");
		return;
	}

	// Print the name of the device chosen
	VkPhysicalDeviceProperties deviceProperties;
	vkGetPhysicalDeviceProperties(*physicalDevice, &deviceProperties);
	DEBUG("RENDER_FRAMEWORK - VkPhysicalDevice Name: %s", deviceProperties.deviceName);
}

void clearVkPhysicalDevice(VkPhysicalDevice * physicalDevice)
{
	*physicalDevice = VK_NULL_HANDLE; 
}

void getAvailableQueues(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface, VkQueueFlags type, bool presentationSupport, std::vector<Queue> * queues)
{
	std::vector<VkQueueFamilyProperties> queueFamilyProperties = getDeviceQueueProperties(physicalDevice);

	uint32_t count = 0;
	uint32_t currentIndex = 0;
	for (auto& queueProperty : queueFamilyProperties)
	{
		if (queueProperty.queueCount > 0 && type & queueProperty.queueFlags)
		{
			if (presentationSupport == VK_TRUE)
			{
				VkBool32 presentSupport = VK_FALSE;
				vkGetPhysicalDeviceSurfaceSupportKHR(physicalDevice, currentIndex, surface, &presentSupport);
				
				if (presentSupport == VK_FALSE)
				{
					currentIndex++;
					continue;
				}
			}
			count++;
		}
		currentIndex++;
	}

	if (count <= 0)
	{
		PANIC("RENDER_FRAMEWORK - Failed to find suitable Queues");
		return;
	}

	queues->resize(count);

	currentIndex = 0;
	int i = 0;
	for (auto& queueProperty : queueFamilyProperties)
	{
		if (queueProperty.queueCount > 0 && type & queueProperty.queueFlags)
		{
			if (presentationSupport == VK_TRUE)
			{
				VkBool32 presentSupport = VK_FALSE;
				vkGetPhysicalDeviceSurfaceSupportKHR(physicalDevice, currentIndex, surface, &presentSupport);
				
				if (presentSupport == VK_FALSE)
				{
					currentIndex++;
					continue;
				}
			}
			(*queues)[i].flags = queueProperty.queueFlags;
			(*queues)[i].index = currentIndex;
			i++;
		}
		currentIndex++;
	}
}

// Takes a VkPhysicalDevice and returns a vector containing the properties of all extensions
// supported by that device.
std::vector<VkExtensionProperties> getDeviceExtensions(VkPhysicalDevice device)
{
	uint32_t count = 0;
	vkEnumerateDeviceExtensionProperties(device, nullptr, &count, nullptr);
	std::vector<VkExtensionProperties> extensionProperties(count);
	vkEnumerateDeviceExtensionProperties(device, nullptr, &count, extensionProperties.data());

	return extensionProperties;
}

// Takes a VkPhysicalDevice and returns a vector containing the properties of all
// queues supported by that device
std::vector<VkQueueFamilyProperties> getDeviceQueueProperties(VkPhysicalDevice device)
{
	uint32_t count = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(device, &count, nullptr);
	std::vector<VkQueueFamilyProperties> queueProperties(count);
	vkGetPhysicalDeviceQueueFamilyProperties(device, &count, queueProperties.data());

	return queueProperties;
}

void createVkDevice(VkPhysicalDevice physicalDevice, uint32_t queueCount, uint32_t * queueIndices, VkDevice * device)
{
	std::vector<VkDeviceQueueCreateInfo> queueInfos(queueCount);

	float priority = 1.0f;
	for (int i = 0; i < queueCount; i++)
	{
		queueInfos[i].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		queueInfos[i].pNext = nullptr;
		queueInfos[i].flags = 0;
		queueInfos[i].queueFamilyIndex = queueIndices[i];
		queueInfos[i].queueCount = 1;
		queueInfos[i].pQueuePriorities = &priority;
	}

	VkPhysicalDeviceFeatures deviceFeatures = {};
	deviceFeatures.samplerAnisotropy = VK_TRUE;

	VkDeviceCreateInfo createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	createInfo.pNext = nullptr;
	createInfo.flags = 0;
	createInfo.queueCreateInfoCount = queueInfos.size();
	createInfo.pQueueCreateInfos = queueInfos.data();
	createInfo.enabledLayerCount = validationLayers.size();
	createInfo.ppEnabledLayerNames = validationLayers.data();
	createInfo.enabledExtensionCount = deviceExtensions.size();
	createInfo.ppEnabledExtensionNames = deviceExtensions.data();
	createInfo.pEnabledFeatures = &deviceFeatures;

	int result = vkCreateDevice(physicalDevice, &createInfo, nullptr, device);
	if (result != VK_SUCCESS)
	{
		PANIC("RENDER_FRAMEWORK - Failed to create Vulkan Logical Device! error_code: %d", result);
		return;
	}

	DEBUG("RENDER_FRAMEWORK - VkDevice Created");
}

void destroyVkDevice(VkDevice * device)
{
	vkDestroyDevice(*device, nullptr);
	
	DEBUG("RENDER_FRAMEWORK - VkDevice Destroyed");
}

void getVkQueue(VkDevice device, uint32_t queueIndex, VkQueue * queue)
{
	vkGetDeviceQueue(device, queueIndex, 0, queue);
}

void createVkCommandPool(VkDevice device, const void * pNext, VkCommandPoolCreateFlags flags,
                         uint32_t queueFamilyIndex, VkCommandPool * commandPool)
{
	VkCommandPoolCreateInfo poolInfo = {};
	poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	poolInfo.pNext = pNext;
	poolInfo.flags = flags;
	poolInfo.queueFamilyIndex = queueFamilyIndex;

	int result = vkCreateCommandPool(device, &poolInfo, nullptr, commandPool);
	if (result != VK_SUCCESS)
	{
		PANIC("RENDER_FRAMEWORK - Failed to create command pool %d", result);
		return;
	}
}

void allocateVkCommandBuffers(VkDevice device, const void * pNext, VkCommandPool commandPool,
                            VkCommandBufferLevel level, uint32_t commandBufferCount,
                            VkCommandBuffer * commandBuffers)
{
	VkCommandBufferAllocateInfo allocInfo = {};
	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.pNext = pNext;
	allocInfo.commandPool = commandPool;
	allocInfo.level = level;
	allocInfo.commandBufferCount = commandBufferCount;

	int result = vkAllocateCommandBuffers(device, &allocInfo, commandBuffers);
	if (result != VK_SUCCESS)
	{
		PANIC("RENDER_FRAMEWORK - Failed to allocate command buffers %d", result);
	}
}

void createVkFence(VkDevice device, const void * pNext, VkFenceCreateFlags flags, VkFence * fence)
{
	VkFenceCreateInfo createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	createInfo.pNext = pNext;
	createInfo.flags = flags;

	int result = vkCreateFence(device, &createInfo, nullptr, fence);
	if (result != VK_SUCCESS)
	{
		PANIC("RENDER_FRAMEWORK - Failed to create fences %d", result);
	}
}

void getSwapchainFormats(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface, VkFormat * colorFormat, VkColorSpaceKHR * colorSpace, VkFormat * depthFormat)
{
	std::vector<VkFormat> colorCandidates = {VK_FORMAT_B8G8R8A8_UNORM, VK_FORMAT_R8G8B8A8_UNORM};
	std::vector<VkFormat> depthCandidates = {VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT};

	if (colorFormat != nullptr)
	{
		*colorFormat = VK_FORMAT_UNDEFINED;

		if (surface != VK_NULL_HANDLE)
		{
			uint32_t formatCount;
			vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &formatCount, nullptr);
			VkSurfaceFormatKHR surfaceFormats[formatCount];
			vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &formatCount, surfaceFormats);

			*colorFormat = surfaceFormats[0].format;
			for (auto& colorCandidate : colorCandidates)
			{
				bool found = false;
				for (int i = 0; i < formatCount; i++)
				{
					if (surfaceFormats[i].format == colorCandidate)
					{
						*colorFormat = surfaceFormats[i].format;
						if (colorSpace != nullptr)
						{
							*colorSpace = surfaceFormats[i].colorSpace;
						}
						found = true;
						break;
					}
				}
				if (found)
				{
					break;
				}
			}
		}
		else
		{
			for (auto& colorCandidate : colorCandidates)
			{
				VkFormatProperties props = {};
				vkGetPhysicalDeviceFormatProperties(physicalDevice, colorCandidate, &props);

				if (props.linearTilingFeatures | props.optimalTilingFeatures |props.bufferFeatures != 0)
				{
					*colorFormat = colorCandidate;
					break;
				}
			}
		}

		if (*colorFormat == VK_FORMAT_UNDEFINED)
		{
			PANIC("RENDER_FRAMEWORK - Failed to find valid color image format");
		}
	}

	if (depthFormat != nullptr)
	{
		*depthFormat = VK_FORMAT_UNDEFINED;

		for (auto& depthCandidate : depthCandidates)
		{
			VkFormatProperties props = {};
			vkGetPhysicalDeviceFormatProperties(physicalDevice, depthCandidate, &props);

			if (props.linearTilingFeatures | props.optimalTilingFeatures |props.bufferFeatures != 0)
			{
				*depthFormat = depthCandidate;
				break;
			}
		}

		if (*depthFormat == VK_FORMAT_UNDEFINED)
		{
			PANIC("RENDER_FRAMEWORK - Failed to find valid depth image format");
		}
	}
}

void getSwapchainPresentMode(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface, VkPresentModeKHR * presentMode)
{
	std::vector<VkPresentModeKHR> presentCandidates = {VK_PRESENT_MODE_MAILBOX_KHR, VK_PRESENT_MODE_FIFO_KHR};

	uint32_t presentModeCount;
	vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, &presentModeCount, nullptr);
	if (presentModeCount == 0)
	{
		PANIC("RENDER_FRAMEWORK - Failed to find a surface present mode");
		return;
	}

	VkPresentModeKHR presentModes[presentModeCount];
	vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, &presentModeCount, presentModes);

	*presentMode = VK_PRESENT_MODE_FIFO_KHR;
	for (auto& candidate : presentCandidates)
	{
		bool found = false;
		for (int i = 0; i < presentModeCount; i++)
		{
			if (presentModes[i] == candidate)
			{
				*presentMode = candidate;
				found = true;
				break;
			}
		}
		if (found)
		{
			break;
		}
	}
}

void getSwapchainExtent(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface, VkExtent2D * extent)
{
	VkSurfaceCapabilitiesKHR surfaceCapabilities = {};
	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, surface, &surfaceCapabilities);
	*extent = surfaceCapabilities.currentExtent;
}

void getSwapchainLength(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface, uint32_t * length)
{
	VkSurfaceCapabilitiesKHR surfaceCapabilities = {};
	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, surface, &surfaceCapabilities);

	if (surfaceCapabilities.maxImageCount != 0)
		*length = std::min(surfaceCapabilities.minImageCount + 1, surfaceCapabilities.maxImageCount);
	else
		*length = surfaceCapabilities.minImageCount + 1;
}

void createVkSwapchainKHR(VkPhysicalDevice physicalDevice, VkDevice device, VkSurfaceKHR surface,
                          uint32_t minImageCount, VkFormat imageFormat, VkColorSpaceKHR imageColorSpace,
                          VkExtent2D imageExtent, uint32_t * pQueueFamilyIndices, uint32_t queueCount,
                          VkPresentModeKHR presentMode, VkSwapchainKHR * swapchain)
{
	VkSurfaceCapabilitiesKHR capabilities;
	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, surface, &capabilities);

	VkBool32 supportsPresentation = VK_FALSE;
	for (int i = 0; i < queueCount; i++)
	{
		vkGetPhysicalDeviceSurfaceSupportKHR(physicalDevice, pQueueFamilyIndices[i], surface, &supportsPresentation);

		if (supportsPresentation == VK_TRUE)
		{
			break;
		}
	}

	if (supportsPresentation == VK_FALSE)
	{
		PANIC("RENDER_FRAMEWORK - Current Surface does not support presentation");
		return;
	}

	createVkSwapchainKHR(device, nullptr, 0, surface, minImageCount, imageFormat, imageColorSpace,
	                     imageExtent, 1, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
	                     (queueCount > 1) ? VK_SHARING_MODE_CONCURRENT : VK_SHARING_MODE_EXCLUSIVE,
	                     queueCount, pQueueFamilyIndices, capabilities.currentTransform,
	                     VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR, presentMode, VK_TRUE, VK_NULL_HANDLE, swapchain);
}

void createVkSwapchainKHR(VkDevice device,
                          const void * pNext,
                          VkSwapchainCreateFlagsKHR flags,
                          VkSurfaceKHR surface,
                          uint32_t minImageCount,
                          VkFormat imageFormat,
                          VkColorSpaceKHR imageColorSpace,
                          VkExtent2D imageExtent,
                          uint32_t imageArrayLayers,
                          VkImageUsageFlags imageUsage,
                          VkSharingMode imageSharingMode,
                          uint32_t queueFamilyIndexCount,
                          const uint32_t * pQueueFamilyIndices,
                          VkSurfaceTransformFlagBitsKHR preTransform,
                          VkCompositeAlphaFlagBitsKHR compositeAlpha,
                          VkPresentModeKHR presentMode,
                          VkBool32 clipped,
                          VkSwapchainKHR oldSwapchain,
                          VkSwapchainKHR * swapchain)
{
	VkSwapchainCreateInfoKHR createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	createInfo.pNext = pNext;
	createInfo.flags = flags;
	createInfo.surface = surface;
	createInfo.minImageCount = minImageCount;
	createInfo.imageFormat = imageFormat;
	createInfo.imageColorSpace = imageColorSpace;
	createInfo.imageExtent = imageExtent;
	createInfo.imageArrayLayers = imageArrayLayers;
	createInfo.imageUsage = imageUsage;
	createInfo.imageSharingMode = imageSharingMode;
	createInfo.queueFamilyIndexCount = queueFamilyIndexCount;
	createInfo.pQueueFamilyIndices = pQueueFamilyIndices;
	createInfo.preTransform = preTransform;
	createInfo.compositeAlpha = compositeAlpha;
	createInfo.presentMode = presentMode;
	createInfo.clipped = clipped;
	createInfo.oldSwapchain = oldSwapchain;

	if (vkCreateSwapchainKHR(device, &createInfo, nullptr, swapchain) != VK_SUCCESS)
	{
		PANIC("RENDER_FRAMEWORK - Failed to create VkSwapchainKHR!");
		return;
	}

	DEBUG("RENDER_FRAMEWORK - VkSwapchainKHR Created");
}

void destroyVkSwapchainKHR(VkDevice logicalDevice, VkSwapchainKHR * swapchain)
{
	vkDestroySwapchainKHR(logicalDevice, *swapchain, nullptr);

	DEBUG("RENDER_FRAMEWORK - VkSwapchainKHR Destroyed");
}

void createVkRenderPass(VkDevice device, VkFormat colorFormat, VkFormat depthFormat, VkSampleCountFlagBits sampleCount, VkAttachmentLoadOp loadOp, std::vector<VkSubpassDependency> dependencies, VkRenderPass * renderPass)
{
	std::vector<VkAttachmentDescription> attachments;

	VkAttachmentReference colorAttachmentRef = {};
	VkAttachmentReference depthAttachmentRef = {};
	VkAttachmentReference resolveAttachmentRef = {};

	VkAttachmentDescription colorAttachment = {};
	VkAttachmentDescription depthAttachment = {};
	VkAttachmentDescription resolveAttachment = {};

	if (colorFormat != VK_FORMAT_UNDEFINED)
	{
		colorAttachment.flags = 0;
		colorAttachment.format = colorFormat;
		colorAttachment.samples = sampleCount;
		colorAttachment.loadOp = loadOp;
		colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		colorAttachment.finalLayout = (sampleCount == VK_SAMPLE_COUNT_1_BIT) ? VK_IMAGE_LAYOUT_PRESENT_SRC_KHR : VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		colorAttachmentRef.attachment = attachments.size();
		colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		attachments.push_back(colorAttachment);
	}
	if (depthFormat != VK_FORMAT_UNDEFINED)
	{
		depthAttachment.flags = 0;
		depthAttachment.format = depthFormat;
		depthAttachment.samples = sampleCount;
		depthAttachment.loadOp = loadOp;
		depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

		depthAttachmentRef.attachment = attachments.size();
		depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

		attachments.push_back(depthAttachment);
	}
	if (sampleCount != VK_SAMPLE_COUNT_1_BIT)
	{
		resolveAttachment.flags = 0;
		resolveAttachment.format = colorFormat;
		resolveAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
		resolveAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		resolveAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		resolveAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		resolveAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		resolveAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		resolveAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

		resolveAttachmentRef.attachment = attachments.size();
		resolveAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		attachments.push_back(resolveAttachment);
	}

	VkSubpassDescription subpass = {};
	subpass.flags = 0;
	subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpass.inputAttachmentCount = 0;
	subpass.pInputAttachments = nullptr;
	subpass.colorAttachmentCount = (colorFormat != VK_FORMAT_UNDEFINED) ? 1 : 0;
	subpass.pColorAttachments = (colorFormat != VK_FORMAT_UNDEFINED) ? &colorAttachmentRef : nullptr;
	subpass.pResolveAttachments = (sampleCount != VK_SAMPLE_COUNT_1_BIT) ? &resolveAttachmentRef : nullptr;
	subpass.pDepthStencilAttachment = (depthFormat != VK_FORMAT_UNDEFINED) ? &depthAttachmentRef : nullptr;
	subpass.preserveAttachmentCount = 0;
	subpass.pPreserveAttachments = nullptr;

	VkRenderPassCreateInfo renderPassInfo = {};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	renderPassInfo.pNext = nullptr;
	renderPassInfo.flags = 0;
	renderPassInfo.attachmentCount = attachments.size();
	renderPassInfo.pAttachments = attachments.data();
	renderPassInfo.subpassCount = 1;
	renderPassInfo.pSubpasses = &subpass;
	renderPassInfo.dependencyCount = dependencies.size();
	renderPassInfo.pDependencies = dependencies.data();

	int result = vkCreateRenderPass(device, &renderPassInfo, nullptr, renderPass);
	if(result != VK_SUCCESS)
	{
		PANIC("RENDER_FRAMEWORK - Failed to create RenderPass - error code: %d", result);
		return;
	}

	DEBUG("RENDER_FRAMEWORK - VkRenderPass Created");
}

void createVkRenderPass(VkDevice device, VkFormat colorFormat, VkFormat depthFormat, VkSampleCountFlagBits sampleCount, VkRenderPass * renderPass)
{
	std::vector<VkSubpassDependency> dependencies(1);

	dependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;
	dependencies[0].dstSubpass = 0;
	dependencies[0].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependencies[0].dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependencies[0].srcAccessMask = 0;
	dependencies[0].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
	dependencies[0].dependencyFlags = 0;

	createVkRenderPass(device, colorFormat, depthFormat, sampleCount, VK_ATTACHMENT_LOAD_OP_CLEAR, dependencies, renderPass);
}

void createVkRenderPassOverlay(VkDevice device, VkFormat colorFormat, VkRenderPass * renderPass)
{
	std::vector<VkSubpassDependency> dependencies(1);

	dependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;
	dependencies[0].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependencies[0].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
	dependencies[0].dstSubpass = 0;
	dependencies[0].dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependencies[0].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
	dependencies[0].dependencyFlags = 0;

	createVkRenderPass(device, colorFormat, VK_FORMAT_UNDEFINED, VK_SAMPLE_COUNT_1_BIT, VK_ATTACHMENT_LOAD_OP_LOAD, dependencies, renderPass);
}

void destroyVkRenderPass(VkDevice device, VkRenderPass * renderPass)
{
	vkDestroyRenderPass(device, *renderPass, nullptr);

	DEBUG("RENDER_FRAMEWORK - VkRenderPass Destroyed");
}

void parseVrApiExtensionsList(char * extensionNames, const char * extensions[], uint32_t * extensionsCount)
{
	char * extensionIterator = extensionNames;
	int i = 0;

	DEBUG("==Extensions==");

	extensions[i++] = extensionNames;
	while (*extensionIterator != '\0')
	{
		if (*extensionIterator == ' ')
		{
			*extensionIterator = '\0';
			extensions[i++] = extensionIterator + 1;
		}
		extensionIterator++;
	}
	*extensionsCount = i;

	for (int k = 0; k < *extensionsCount; k++)
	{
		DEBUG("%s", extensions[k]);
	}
}

void createVkImage(Context * context,
                   VkImageType imageType,
                   VkFormat format,
                   VkExtent2D extent,
                   uint32_t miplevels,
                   uint32_t arrayLayers,
                   VkSampleCountFlagBits samples,
                   VkImageTiling tiling,
                   VkImageUsageFlags usage,
                   VkMemoryPropertyFlags properties,
                   VkImage * image,
                   VkDeviceMemory * imageMemory)
{
	VkExtent3D e = {};
	e.width = extent.width;
	e.height = extent.height;
	e.depth = 1;

	createVkImage(context, imageType, format, e, miplevels, arrayLayers, samples,
	              tiling, usage, properties, image, imageMemory);
}

void createVkImage(Context * context,
                   VkImageType imageType,
                   VkFormat format,
                   VkExtent3D extent,
                   uint32_t miplevels,
                   uint32_t arrayLayers,
                   VkSampleCountFlagBits samples,
                   VkImageTiling tiling,
                   VkImageUsageFlags usage,
                   VkMemoryPropertyFlags properties,
                   VkImage * image,
                   VkDeviceMemory * imageMemory)
{

	uint32_t queueIndices [] = {context->graphicsQueue.index, context->transferQueue.index};

	const void * pNext = nullptr;
	uint32_t flags = (arrayLayers == 6) ? VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT : 0;
	VkSharingMode sharingMode = (context->queueCount > 1) ? VK_SHARING_MODE_CONCURRENT : VK_SHARING_MODE_EXCLUSIVE;
	uint32_t queueFamilyIndexCount = context->queueCount;
	const uint32_t * pQueueFamilyIndices = queueIndices;
	VkImageLayout initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

	createVkImage(context->physicalDevice, context->device, pNext, flags, imageType, format, extent,
	              miplevels, arrayLayers, samples, tiling, usage, sharingMode, queueFamilyIndexCount,
	              pQueueFamilyIndices, initialLayout, properties, image, imageMemory);
}

void createVkImage(VkPhysicalDevice physicalDevice,
                   VkDevice device,
                   VkImageType imageType,
                   VkFormat format,
                   VkExtent2D extent,
                   uint32_t miplevels,
                   uint32_t arrayLayers,
                   VkSampleCountFlagBits samples,
                   VkImageTiling tiling,
                   VkImageUsageFlags usage,
                   VkSharingMode sharingMode,
                   uint32_t queueFamilyIndexCount,
                   const uint32_t * pQueueFamilyIndices,
                   VkMemoryPropertyFlags properties,
                   VkImage * image,
                   VkDeviceMemory * imageMemory)
{
	VkExtent3D e = {};
	e.width = extent.width;
	e.height = extent.height;
	e.depth = 1;

	createVkImage(physicalDevice, device, imageType, format, e, miplevels,
	              arrayLayers, samples, tiling, usage, sharingMode, queueFamilyIndexCount,
	              pQueueFamilyIndices, properties, image, imageMemory);
}

void createVkImage(VkPhysicalDevice physicalDevice,
                   VkDevice device,
                   VkImageType imageType,
                   VkFormat format,
                   VkExtent3D extent,
                   uint32_t miplevels,
                   uint32_t arrayLayers,
                   VkSampleCountFlagBits samples,
                   VkImageTiling tiling,
                   VkImageUsageFlags usage,
                   VkSharingMode sharingMode,
                   uint32_t queueFamilyIndexCount,
                   const uint32_t * pQueueFamilyIndices,
                   VkMemoryPropertyFlags properties,
                   VkImage * image,
                   VkDeviceMemory * imageMemory)
{
	const void * pNext = nullptr;
	uint32_t flags = (arrayLayers == 6) ? VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT : 0;
	VkImageLayout initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

	createVkImage(physicalDevice, device, pNext, flags, imageType, format, extent, miplevels,
	              arrayLayers, samples, tiling, usage, sharingMode, queueFamilyIndexCount,
	              pQueueFamilyIndices, initialLayout, properties, image, imageMemory);
}

void createVkImage(VkPhysicalDevice physicalDevice,
                   VkDevice device,
                   const void * pNext,
                   VkImageCreateFlags flags,
                   VkImageType imageType,
                   VkFormat format,
                   VkExtent3D extent,
                   uint32_t miplevels,
                   uint32_t arrayLayers,
                   VkSampleCountFlagBits samples,
                   VkImageTiling tiling,
                   VkImageUsageFlags usage,
                   VkSharingMode sharingMode,
                   uint32_t queueFamilyIndexCount,
                   const uint32_t * pQueueFamilyIndices,
                   VkImageLayout initialLayout,
                   VkMemoryPropertyFlags properties,
                   VkImage * image,
                   VkDeviceMemory * imageMemory)
{
	VkImageCreateInfo createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	createInfo.pNext = pNext;
	createInfo.flags = flags;
	createInfo.imageType = imageType;
	createInfo.format = format;
	createInfo.extent = extent;
	createInfo.mipLevels = miplevels;
	createInfo.arrayLayers = arrayLayers;
	createInfo.samples = samples;
	createInfo.tiling = tiling;
	createInfo.usage = usage;
	createInfo.sharingMode = sharingMode;
	createInfo.queueFamilyIndexCount = queueFamilyIndexCount;
	createInfo.pQueueFamilyIndices = pQueueFamilyIndices;
	createInfo.initialLayout = initialLayout;

	int result = vkCreateImage(device, &createInfo, nullptr, image);
	if (result != VK_SUCCESS)
	{
		PANIC("RENDER_FRAMEWORK - Failed to create VkImage %d", result);
		return;
	}

	VkMemoryRequirements memReqs;
	vkGetImageMemoryRequirements(device, *image, &memReqs);

	VkPhysicalDeviceMemoryProperties memProps;
	vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memProps);

	uint32_t typeIndex = -1;
	for (uint32_t i = 0; i < memProps.memoryTypeCount; i++)
	{
		if (memReqs.memoryTypeBits & (1 << i) && (memProps.memoryTypes[i].propertyFlags & properties) == properties)
		{
			typeIndex = i;
		}
	}

	if (typeIndex == -1)
	{
		PANIC("RENDER_FRAMEWORK - Failed to allocate VkImage memory");
		vkDestroyImage(device, *image, nullptr);
		return;
	}

	VkMemoryAllocateInfo allocInfo = {};
	allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocInfo.pNext = nullptr;
	allocInfo.allocationSize = memReqs.size;
	allocInfo.memoryTypeIndex = typeIndex;

	result = vkAllocateMemory(device, &allocInfo, nullptr, imageMemory);
	if (result != VK_SUCCESS)
	{
		PANIC("RENDER_FRAMEWORK - Failed to allocate image memory! %d", result);
	}

	vkBindImageMemory(device, *image, *imageMemory, 0);
}

void createVkImageView(Context * context,
                       VkImage image,
                       VkFormat format,
                       uint32_t miplevels,
                       uint32_t arrayLayers,
                       VkImageAspectFlagBits aspectFlags,
                       VkImageView * imageView)
{
	createVkImageView(context->physicalDevice, context->device, image,
	                  (arrayLayers == 6) ? VK_IMAGE_VIEW_TYPE_CUBE : VK_IMAGE_VIEW_TYPE_2D,
	                  format, miplevels, arrayLayers, aspectFlags, imageView);
}

void createVkImageView(VkPhysicalDevice physicalDevice,
                       VkDevice device,
                       VkImage image,
                       VkImageViewType viewType,
                       VkFormat format,
                       uint32_t miplevels,
                       uint32_t arrayLayers,
                       VkImageAspectFlagBits aspectFlags,
                       VkImageView * imageView)
{
	const void * pNext = nullptr;
	VkImageViewCreateFlags flags = 0;
	VkComponentMapping components = {};
	components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
	components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
	components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
	components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
	VkImageSubresourceRange subresourceRange = {};
	subresourceRange.aspectMask = aspectFlags;
	subresourceRange.baseMipLevel = 0;
	subresourceRange.levelCount = miplevels;
	subresourceRange.baseArrayLayer = 0;
	subresourceRange.layerCount = arrayLayers;

	createVkImageView(physicalDevice, device, pNext, flags, image, viewType, format, components,
	                  subresourceRange, imageView);
}

void createVkImageView(VkPhysicalDevice physicalDevice,
                       VkDevice device,
                       const void * pNext,
                       VkImageViewCreateFlags flags,
                       VkImage image,
                       VkImageViewType viewType,
                       VkFormat format,
                       VkComponentMapping components,
                       VkImageSubresourceRange subresourceRange,
                       VkImageView * imageView)
{
	VkImageViewCreateInfo createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	createInfo.pNext = pNext;
	createInfo.flags = flags;
	createInfo.image = image;
	createInfo.viewType = viewType;
	createInfo.format = format;
	createInfo.components = components;
	createInfo.subresourceRange = subresourceRange;

	int result = vkCreateImageView(device, &createInfo, nullptr, imageView);
	if (result != VK_SUCCESS)
	{
		PANIC("RENDER_FRAMEWORK - Failed to create VkImageView! %d", result);
	}
}

void createVkFramebuffer(VkDevice device, const void * pNext, VkFramebufferCreateFlags flags,
                         VkRenderPass renderPass, VkImageView colorImageView,
                         VkImageView depthImageView, VkImageView swapchainImageView, uint32_t width,
                         uint32_t height, uint32_t layers, VkFramebuffer * framebuffer)
{
	std::vector<VkImageView> attachments;

	if (colorImageView != VK_NULL_HANDLE)
		attachments.push_back(colorImageView);

	if (depthImageView != VK_NULL_HANDLE)
		attachments.push_back(depthImageView);

	if (swapchainImageView != VK_NULL_HANDLE)
		attachments.push_back(swapchainImageView);

	VkFramebufferCreateInfo createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
	createInfo.pNext = pNext;
	createInfo.flags = flags;
	createInfo.renderPass = renderPass;
	createInfo.attachmentCount = attachments.size();
	createInfo.pAttachments = attachments.data();
	createInfo.width = width;
	createInfo.height = height;
	createInfo.layers = layers;

	int result = vkCreateFramebuffer(device, &createInfo, nullptr, framebuffer);
	if (result != VK_SUCCESS)
	{
		PANIC("RENDER_FRAMEWORK _ Failed to create framebuffer %d", result);
	}
}

VkCommandBuffer beginSingleTimeCommands(Context * context)
{
	VkCommandBufferAllocateInfo allocInfo = {};
	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocInfo.commandPool = context->transferCommandPool;
	allocInfo.commandBufferCount = 1;

	VkCommandBuffer commandBuffer;
	vkAllocateCommandBuffers(context->device, &allocInfo, &commandBuffer);

	VkCommandBufferBeginInfo beginInfo = {};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

	vkBeginCommandBuffer(commandBuffer, &beginInfo);

	return commandBuffer;
}

void endSingleTimeCommands(Context * context, VkCommandBuffer commandBuffer)
{
	vkEndCommandBuffer(commandBuffer);

	VkSubmitInfo submitInfo = {};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &commandBuffer;

	vkQueueSubmit(context->transferQueue.queue, 1, &submitInfo, VK_NULL_HANDLE);
	vkQueueWaitIdle(context->transferQueue.queue);

	vkFreeCommandBuffers(context->device, context->transferCommandPool, 1, &commandBuffer);
}

void transitionImageLayout(Context * context, VkImage image, VkFormat format,
                           VkImageLayout oldLayout, VkImageLayout newLayout, uint32_t layers)
{
	VkCommandBuffer commandBuffer = beginSingleTimeCommands(context);

	VkImageMemoryBarrier barrier = {};
	barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	barrier.oldLayout = oldLayout;
	barrier.newLayout = newLayout;
	barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.image = image;

	if (newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL)
	{
		barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;

		if (format == VK_FORMAT_D32_SFLOAT_S8_UINT || format == VK_FORMAT_D24_UNORM_S8_UINT)
		{
			barrier.subresourceRange.aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;
		}
	}
	else
	{
		barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	}

	barrier.subresourceRange.baseMipLevel = 0;
	barrier.subresourceRange.levelCount = 1;
	barrier.subresourceRange.baseArrayLayer = 0;
	barrier.subresourceRange.layerCount = layers;

	VkPipelineStageFlags sourceStage;
	VkPipelineStageFlags destinationStage;

	if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
	{
		barrier.srcAccessMask = 0;
		barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

		sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
		destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
	}
	else if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL)
	{
		barrier.srcAccessMask = 0;
		barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;

		sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
		destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
	}
	else if (oldLayout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
	{
		barrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT |
		                        VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
		barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

		sourceStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
	}
	else if (oldLayout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL)
	{
		barrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT |
		                        VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
		barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;

		sourceStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
	}
	else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL)
	{
		barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		barrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT |
		                        VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

		sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
		destinationStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	}
	else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL)
	{
		barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
		barrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT |
		                        VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

		sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
		destinationStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	}
	else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
	{
		barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

		sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
		destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
	}
	else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
	{
		barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
		barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

		sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
		destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
	}
	else if (oldLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL)
	{
		barrier.srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
		barrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT |
		                        VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

		sourceStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
		destinationStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	}
	else if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL)
	{
		barrier.srcAccessMask = 0;
		barrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT |
		                        VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

		sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
		destinationStage = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
	}
	else if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL)
	{
		barrier.srcAccessMask = 0;
		barrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT |
		                        VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

		sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
		destinationStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	}
	else
	{
		PANIC("RENDER_FRAMEWORK - Unsupported layout transition");
	}

	vkCmdPipelineBarrier(
		commandBuffer,
		sourceStage, destinationStage,
		0,
		0, nullptr,
		0, nullptr,
		1, &barrier
	);

	endSingleTimeCommands(context, commandBuffer); 
}

void createBuffer(Context * context, VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties,
                  VkBuffer * buffer, VkDeviceMemory * bufferMemory)
{
	VkBufferCreateInfo bufferInfo = {};
	bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	bufferInfo.size = size;
	bufferInfo.usage = usage;

	uint32_t queueIndices [] = {context->graphicsQueue.index, context->transferQueue.index};

	bufferInfo.queueFamilyIndexCount = context->queueCount;
	bufferInfo.pQueueFamilyIndices = queueIndices;
	bufferInfo.sharingMode = (context->queueCount > 1) ? VK_SHARING_MODE_CONCURRENT : VK_SHARING_MODE_EXCLUSIVE;

	if (vkCreateBuffer(context->device, &bufferInfo, nullptr, buffer) != VK_SUCCESS)
	{
		PANIC("Failed to create buffer!\n");
	}

	VkMemoryRequirements memReqs;
	vkGetBufferMemoryRequirements(context->device, *buffer, &memReqs);

	VkPhysicalDeviceMemoryProperties memProps;
	vkGetPhysicalDeviceMemoryProperties(context->physicalDevice, &memProps);

	uint32_t typeIndex = -1;
	for (uint32_t i = 0; i < memProps.memoryTypeCount; i++)
	{
		if (memReqs.memoryTypeBits & (1 << i) && (memProps.memoryTypes[i].propertyFlags & properties) == properties)
		{
			typeIndex = i;
		}
	}

	VkMemoryAllocateInfo allocInfo = {};
	allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocInfo.allocationSize = memReqs.size;
	allocInfo.memoryTypeIndex = typeIndex;

	if (vkAllocateMemory(context->device, &allocInfo, nullptr, bufferMemory) != VK_SUCCESS)
	{
		PANIC("Failed to allocate memory for buffer!\n");
	}

	vkBindBufferMemory(context->device, *buffer, *bufferMemory, 0);
}

void copyBuffer(Context * context, VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size)
{
	VkCommandBuffer commandBuffer = beginSingleTimeCommands(context);

	VkBufferCopy copyRegion = {};
	copyRegion.srcOffset = 0;
	copyRegion.dstOffset = 0;
	copyRegion.size = size;
	vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);

	endSingleTimeCommands(context, commandBuffer);
}

void copyBufferToImage(Context * context, VkBuffer buffer, VkImage image, uint32_t width, uint32_t height, uint32_t layerCount)
{
	VkCommandBuffer commandBuffer = beginSingleTimeCommands(context);

	VkBufferImageCopy region = {};
	region.bufferOffset = 0;
	region.bufferRowLength = 0;
	region.bufferImageHeight = 0;

	region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	region.imageSubresource.mipLevel = 0;
	region.imageSubresource.baseArrayLayer = 0;
	region.imageSubresource.layerCount = layerCount;

	region.imageOffset = {0, 0, 0};
	region.imageExtent = {width, height, 1};

	vkCmdCopyBufferToImage(commandBuffer, buffer, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);

	endSingleTimeCommands(context, commandBuffer);
}

void loadOBJ(std::string filename, std::string location, std::vector<Vertex> * vertices,
             std::vector<uint32_t> * indices, std::vector<std::string> * diffuseTextureNames)
{
	int totalVerts, face;

	std::string err, warn;

	tinyobj::attrib_t attrib;
	std::vector<tinyobj::shape_t> shapes;
	std::vector<tinyobj::material_t> materials;

	tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, (location + "/" + filename).c_str(), location.c_str(), true, true);

	std::unordered_map<Vertex, uint32_t> uniqueVertices;

	// TODO: Cleanup TexID and investigate texCoords errors
	// Some texID values are garbage

	for (auto& shape : shapes)
	{
		size_t index_offset = 0;
		for (size_t f = 0; f < shape.mesh.num_face_vertices.size(); f++)
		{
			int fv = shape.mesh.num_face_vertices[f];
			for (size_t v = 0; v < fv; v++)
			{
				tinyobj::index_t index = shape.mesh.indices[index_offset + v];

				Vertex vertex = {};

				vertex.position = {
					attrib.vertices[3 * index.vertex_index + 0],
					attrib.vertices[3 * index.vertex_index + 1],
					attrib.vertices[3 * index.vertex_index + 2]
				};

				vertex.tex = {
					attrib.texcoords[2 * index.texcoord_index + 0],
					1.0f - attrib.texcoords[2 * index.texcoord_index + 1]
				};

				vertex.normal = {
					attrib.normals[3 * index.normal_index + 0],
					attrib.normals[3 * index.normal_index + 1],
					attrib.normals[3 * index.normal_index + 2]
				};

				vertex.color = {1.0f, 1.0f, 1.0f};

				vertex.texID = diffuseTextureNames->size() + shape.mesh.material_ids[f];

				if (uniqueVertices.count(vertex) == 0)
				{
					uniqueVertices[vertex] = static_cast<uint32_t>(vertices->size());
					vertices->push_back(vertex);
				}

				indices->push_back(uniqueVertices[vertex]);
			}
			index_offset += fv;
		}
	}

	for (auto& material : materials)
	{
		diffuseTextureNames->push_back(material.diffuse_texname);
	}

	if (err.length() > 1)
	{
		PANIC("%s", err.c_str());
	}

	if (warn.length() > 1)
	{
		WARN("%s", warn.c_str());
	}
}

void loadSceneModels(std::string filename, std::vector<Mesh> * meshes, std::vector<std::string> * diffuseTextureNames)
{
	std::ifstream shoppingList(filename);

	if (!shoppingList.is_open())
	{
		throw 117;
		//shoppingList.exceptions(shoppingList.failbit);
		//return;
	}

	std::string location;

	shoppingList >> location;

	std::string err, warn;

	while (true)
	{
		int maxCount = 0;
		std::string objectname;

		shoppingList >> maxCount;
		shoppingList >> objectname;

		if (shoppingList.eof())
			break;

		meshes->emplace_back();

		meshes->back().name = objectname;
		meshes->back().instances.resize(maxCount);

		for (auto& instance : meshes->back().instances)
		{
			instance.material.textureIndex = diffuseTextureNames->size();
		}

		loadOBJ(objectname, location, &meshes->back().vertices, &meshes->back().indices, diffuseTextureNames);
	}

	shoppingList.close();
}

void createMeshTextureSampler(VkDevice device, VkSampler * textureSampler)
{
	VkSamplerCreateInfo samplerInfo = {};
	samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
	samplerInfo.pNext = nullptr;
	samplerInfo.flags = 0;
	samplerInfo.magFilter = VK_FILTER_LINEAR;
	samplerInfo.minFilter = VK_FILTER_LINEAR;
	samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
	samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerInfo.mipLodBias = 0.0f;
	samplerInfo.anisotropyEnable = VK_TRUE;
	samplerInfo.maxAnisotropy = 16;
	samplerInfo.compareEnable = VK_FALSE;
	samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
	samplerInfo.minLod = 0.0f;
	samplerInfo.maxLod = 0.0f;
	samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
	samplerInfo.unnormalizedCoordinates = VK_FALSE;

	int result = vkCreateSampler(device, &samplerInfo, nullptr, textureSampler);
	if (result != VK_SUCCESS)
	{
		PANIC("RENDER_FRAMEWORK - Failed to create texture sampler %d", result);
	}
}

int getFileExtension(const char * filename)
{
	int i, j;

	char extension[10]; 

	enum extensions {
		dds = 0,
		png,
		jpg,
		bmp,
		other
	};

	for (i = 0; filename[i] != '\0'; i++);

	for ( ; filename[i-1] != '.'; i--);

	for (j = 0; filename[i] != '\0'; i++, j++)
		extension[j] = filename[i];

	extension[j] = '\0';

	if (strcmp(extension, "dds") == 0)
		return dds;

	if (strcmp(extension, "png") == 0)
		return png;

	if (strcmp(extension, "jpg") == 0)
		return jpg;

	if (strcmp(extension, "bmp") == 0)
		return bmp;

	return other;
}

void loadMeshTexture(Context * context, Renderer * renderer, const char * textureName,
                     VkImage * image, VkDeviceMemory * imageMemory, VkImageView * imageView)
{
	unsigned char * pixels = nullptr;
	dds_info imageInfo;
	int width, height, channels;
	VkDeviceSize imageSize;
	VkFormat format = VK_FORMAT_R8G8B8A8_UNORM;

	if (getFileExtension(textureName) == 0)
	{
		static const dds_u32 supfmt[] = { DDS_FMT_R8G8B8A8, DDS_FMT_B8G8R8A8, DDS_FMT_B8G8R8X8, DDS_FMT_DXT1, DDS_FMT_DXT3, DDS_FMT_DXT5, 0 };
		int result = dds_load_from_file(textureName, &imageInfo, supfmt);

		if (result != DDS_SUCCESS)
		{
			PANIC("RENDER_FRAMEWORK - Failed to load texture %s %d", textureName, result);
			throw 117;
		}

		width = imageInfo.image.width;
		height = imageInfo.image.height;

		DEBUG("%d %d %d %d", width, height, imageInfo.image.size, imageInfo.image.format);

		pixels = (unsigned char *) malloc(width * height * sizeof(unsigned char));

		if (imageInfo.image.format == DDS_FMT_DXT5)
			format = VK_FORMAT_BC3_UNORM_BLOCK;

		dds_read(&imageInfo, pixels);

		dds_close(&imageInfo);

		imageSize = width * height;
	}
	else
	{
		pixels = stbi_load(textureName, &width, &height, &channels, STBI_rgb_alpha);
		imageSize = width * height * 4;
	}

	if (!pixels)
	{
		PANIC("RENDER_FRAMEWORK - Failed to load texture %s", textureName);
		throw 117;
	}
	VkBuffer stagingBuffer;
	VkDeviceMemory stagingBufferMemory;
	createBuffer(context, imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
	             VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
	             &stagingBuffer, &stagingBufferMemory);

	void * data;
	vkMapMemory(context->device, stagingBufferMemory, 0, imageSize, 0, &data);
	memcpy(((unsigned char *) data), pixels, imageSize);
	vkUnmapMemory(context->device, stagingBufferMemory);

	free(pixels);

	VkExtent2D e = {};
	e.width = width;
	e.height = height;

	createVkImage(context, VK_IMAGE_TYPE_2D, format, e, 1, 1, VK_SAMPLE_COUNT_1_BIT,
	            VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
	            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, image, imageMemory);

	transitionImageLayout(context, *image, format, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1);

	copyBufferToImage(context, stagingBuffer, *image, static_cast<uint32_t>(width), static_cast<uint32_t>(height), 1);

	transitionImageLayout(context, *image, format, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, 1);

	createVkImageView(context, *image, format, 1, 1, VK_IMAGE_ASPECT_COLOR_BIT, imageView);

	vkDestroyBuffer(context->device, stagingBuffer, nullptr);
	vkFreeMemory(context->device, stagingBufferMemory, nullptr);
}

void createVertexBuffer(Context * context, std::vector<Vertex>& vertices, VkBuffer * vertexBuffer,
                        VkDeviceMemory * vertexMemory, VkDeviceSize * vertexBufferSize)
{
	*vertexBufferSize = sizeof(Vertex) * vertices.size();

	VkBuffer stagingBuffer;
	VkDeviceMemory stagingBufferMemory;
	VkBufferUsageFlags usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
	VkMemoryPropertyFlags properties = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
	createBuffer(context, *vertexBufferSize, usage, properties, &stagingBuffer, &stagingBufferMemory);

	void * data;
	vkMapMemory(context->device, stagingBufferMemory, 0, *vertexBufferSize, 0, &data);
	memcpy(data, vertices.data(), (size_t) *vertexBufferSize);
	vkUnmapMemory(context->device, stagingBufferMemory);

	usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
	properties = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
	createBuffer(context, *vertexBufferSize, usage, properties, vertexBuffer, vertexMemory);

	copyBuffer(context, stagingBuffer, *vertexBuffer, *vertexBufferSize);

	vkDestroyBuffer(context->device, stagingBuffer, nullptr);
	vkFreeMemory(context->device, stagingBufferMemory, nullptr);
}

void createIndexBuffer(Context * context, std::vector<uint32_t>& indices, VkBuffer * indexBuffer,
                       VkDeviceMemory * indexMemory, VkDeviceSize * indexBufferSize)
{
	*indexBufferSize = sizeof(uint32_t) * indices.size();

	VkBuffer stagingBuffer;
	VkDeviceMemory stagingBufferMemory;
	VkBufferUsageFlags usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
	VkMemoryPropertyFlags properties = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
	createBuffer(context, *indexBufferSize, usage, properties, &stagingBuffer, &stagingBufferMemory);

	void * data;
	vkMapMemory(context->device, stagingBufferMemory, 0, *indexBufferSize, 0, &data);
	memcpy(data, indices.data(), (size_t) *indexBufferSize);
	vkUnmapMemory(context->device, stagingBufferMemory);

	usage = VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
	properties = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
	createBuffer(context, *indexBufferSize, usage, properties, indexBuffer, indexMemory);

	copyBuffer(context, stagingBuffer, *indexBuffer, *indexBufferSize);

	vkDestroyBuffer(context->device, stagingBuffer, nullptr);
	vkFreeMemory(context->device, stagingBufferMemory, nullptr);
}

void createInstanceBuffer(Context * context, std::vector<Instance>& instances, VkBuffer * instanceBuffer,
                          VkDeviceMemory * instanceMemory, VkDeviceSize * instanceBufferSize)
{
	*instanceBufferSize = sizeof(Instance) * instances.size();

	VkBuffer stagingBuffer;
	VkDeviceMemory stagingBufferMemory;
	VkBufferUsageFlags usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
	VkMemoryPropertyFlags properties = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
	createBuffer(context, *instanceBufferSize, usage, properties, &stagingBuffer, &stagingBufferMemory);

	void * data;
	vkMapMemory(context->device, stagingBufferMemory, 0, *instanceBufferSize, 0, &data);
	memcpy(data, instances.data(), (size_t) *instanceBufferSize);
	vkUnmapMemory(context->device, stagingBufferMemory);

	usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
	properties = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
	createBuffer(context, *instanceBufferSize, usage, properties, instanceBuffer, instanceMemory);

	copyBuffer(context, stagingBuffer, *instanceBuffer, *instanceBufferSize);

	vkDestroyBuffer(context->device, stagingBuffer, nullptr);
	vkFreeMemory(context->device, stagingBufferMemory, nullptr);
}

void createMeshBuffers(Context * context, Mesh * mesh)
{
	createVertexBuffer(context, mesh->vertices, &mesh->vertexBuffer, &mesh->vertexMemory, &mesh->vertexBufferSize);
	createIndexBuffer(context, mesh->indices, &mesh->indexBuffer, &mesh->indexMemory, &mesh->indexBufferSize);
	createInstanceBuffer(context, mesh->instances, &mesh->instanceBuffer, &mesh->instanceMemory, &mesh->instanceBufferSize);
}

void createShaderModule(Context * context, std::string filename, VkShaderModule * shaderModule)
{
	std::ifstream file(filename, std::ios::binary | std::ios::ate);
	std::streamsize size = file.tellg();
	file.seekg(0, std::ios::beg);

	std::vector<char> buffer(size);
	if (!file.read(buffer.data(), size))
	{
		PANIC("RENDER_FRAMEWORK - Failed to read shader file %s", filename.c_str());
		return;
	}

	VkShaderModuleCreateInfo createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	createInfo.codeSize = size;
	createInfo.pCode = reinterpret_cast<const uint32_t*>(buffer.data());
	int result = vkCreateShaderModule(context->device, &createInfo, nullptr, shaderModule); 
	if (result != VK_SUCCESS)
	{
		PANIC("RENDER_FRAMEWORK - Failed to create shader module %d", result);
		return;
	}

	DEBUG("RENDER_FRAMEWORK - VkShaderModule Created %s", filename.c_str());
}

VkDescriptorSetLayoutBinding getDescriptorSetLayoutBinding(uint32_t binding, VkDescriptorType type,
                                                           uint32_t descriptorCount,
                                                           VkShaderStageFlags stageFlags,
                                                           const VkSampler * pImmutableSamplers)
{
	VkDescriptorSetLayoutBinding layoutBinding = {};
	layoutBinding.binding = binding;
	layoutBinding.descriptorType = type;
	layoutBinding.descriptorCount = descriptorCount;
	layoutBinding.stageFlags = stageFlags;
	layoutBinding.pImmutableSamplers = pImmutableSamplers;

	return layoutBinding;
}

void createVkDescriptorPool(VkDevice device, std::vector<VkDescriptorSetLayoutBinding>& bindings,
                            uint32_t maxSets, VkDescriptorPool * descriptorPool)
{
	std::vector<VkDescriptorPoolSize> sizes(bindings.size());

	for (int i = 0; i < sizes.size(); i++)
	{
		sizes[i].type = bindings[i].descriptorType;
		sizes[i].descriptorCount = maxSets * bindings[i].descriptorCount;
	}

	VkDescriptorPoolCreateInfo createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	createInfo.pNext = nullptr;
	createInfo.flags = 0;
	createInfo.maxSets = maxSets;
	createInfo.poolSizeCount = sizes.size();
	createInfo.pPoolSizes = sizes.data();

	int result = vkCreateDescriptorPool(device, &createInfo, nullptr, descriptorPool);
	if (result != VK_SUCCESS)
	{
		PANIC("RENDER_FRAMEWORK - Failed to create Descriptor Pool %d", result);
	}
}

void createVkDescriptorSetLayout(VkDevice device, std::vector<VkDescriptorSetLayoutBinding>& bindings,
                                 VkDescriptorSetLayout * layout)
{
	VkDescriptorSetLayoutCreateInfo createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	createInfo.pNext = nullptr;
	createInfo.flags = 0;
	createInfo.bindingCount = bindings.size();
	createInfo.pBindings = bindings.data();

	int result = vkCreateDescriptorSetLayout(device, &createInfo, nullptr, layout);
	if (result != VK_SUCCESS)
	{
		PANIC("RENDER_FRAMEWORK - Failed to create VkDescriptorSetLayout %d", result);
	}
}

void allocateVkDescriptorSets(VkDevice device, VkDescriptorPool descriptorPool,
                              std::vector<VkDescriptorSetLayout>& layouts,
                              std::vector<VkDescriptorSet> * descriptorSets)
{
	VkDescriptorSetAllocateInfo allocInfo = {};
	allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	allocInfo.pNext = nullptr;
	allocInfo.descriptorPool = descriptorPool;
	allocInfo.descriptorSetCount = layouts.size();
	allocInfo.pSetLayouts = layouts.data();

	descriptorSets->resize(layouts.size());

	int result = vkAllocateDescriptorSets(device, &allocInfo, descriptorSets->data());
	if (result != VK_SUCCESS)
	{
		PANIC("RENDER_FRAMEWORK - Failed to allocate Descriptor Sets %d", result);
	}
}
}
