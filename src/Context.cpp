#include <RenderFramework.h>

using namespace RenderFramework;

Context::Context()
{
#ifndef ANDROID
	createBorderlessWindow(&window);
	glfwSetWindowUserPointer(window, this);
	createInstanceGLFW(validationLayers, instanceExtensions, &instance);
	createVkSurfaceKHR(instance, window, &surface);
#else
	// TODO: Create OVR Instance
	std::vector<const char *> extensions();

	createInstance(nullptr, 0, nullptr, 0, VkInstance * instance);
#endif
	chooseVkPhysicalDevice(instance, &physicalDevice);
	chooseQueues();
	uint32_t queueIndices [] = {graphicsQueue.index, transferQueue.index, presentQueue.index};
	createVkDevice(physicalDevice, queueCount, queueIndices, &device);
	getVkQueue(device, graphicsQueue.index, &graphicsQueue.queue);
	getVkQueue(device, transferQueue.index, &transferQueue.queue);
	getVkQueue(device, presentQueue.index, &presentQueue.queue);
	createVkCommandPool(device, nullptr, 0, graphicsQueue.index, &graphicsCommandPool);

	if (graphicsQueue.index != transferQueue.index)
	{
		createVkCommandPool(device, nullptr, 0, transferQueue.index, &transferCommandPool);
	}
	else
	{
		transferCommandPool = graphicsCommandPool;
	}


	DEBUG("RENDER_FRAMEWORK - Context Created");
}

Context::~Context()
{
	vkDestroyCommandPool(device, graphicsCommandPool, nullptr);
	if (graphicsQueue.index != transferQueue.index)
	{
		vkDestroyCommandPool(device, transferCommandPool, nullptr);
	}
	destroyVkDevice(&device);
	clearVkPhysicalDevice(&physicalDevice);
	destroyVkSurfaceKHR(instance, &surface);
	destroyInstance(&instance);
	destroyWindow(&window);

	DEBUG("RENDER_FRAMEWORK - Context Destroyed");
}

void Context::chooseQueues()
{
	std::vector<Queue> queues;
	getAvailableQueues(physicalDevice, surface, VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_TRANSFER_BIT, VK_TRUE, &queues);
	graphicsQueue = queues[0];
	transferQueue = queues[0];
	presentQueue = queues[0];
	queueCount = 1;

	/*
	std::vector<Queue> graphicsQueues;
	getAvailableQueues(physicalDevice, surface, VK_QUEUE_GRAPHICS_BIT, VK_FALSE, &graphicsQueues);
	graphicsQueue = graphicsQueues[0];

	std::vector<Queue> transferQueues;
	getAvailableQueues(physicalDevice, surface, VK_QUEUE_TRANSFER_BIT, VK_FALSE, &transferQueues);
	transferQueue = transferQueues[0];

	std::vector<Queue> presentQueues;
	getAvailableQueues(physicalDevice, surface, VK_QUEUE_FLAG_BITS_MAX_ENUM, VK_TRUE, &presentQueues);
	presentQueue = presentQueues[0];

	int i = 1;
	int j = 1;
	int k = 1;
	while (i < graphicsQueues.size() && graphicsQueue.index != transferQueue.index)
		graphicsQueue = graphicsQueues[i++];

	while (j < transferQueues.size() && graphicsQueue.index != transferQueue.index)
		transferQueue = transferQueues[j++];

	while (k < presentQueues.size() && graphicsQueue.index != presentQueue.index)
		presentQueue = presentQueues[k++];

	queueCount = 1;

	if (graphicsQueue.index != transferQueue.index)
		queueCount++;

	if (graphicsQueue.index != presentQueue.index)
		queueCount++;
	*/
}
