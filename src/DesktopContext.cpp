#include <render/DesktopContext.h>

// ===============================================================================================================
//                                            GLFWwindow Helpers
// ===============================================================================================================

GLFWwindow * createBorderlessGLFWwindow()
{
    GLFWwindow * window;

    // Setup Borderless Windowed Mode
    GLFWmonitor * monitor = glfwGetPrimaryMonitor();
    const GLFWvidmode * mode = glfwGetVideoMode(monitor);
    glfwWindowHint(GLFW_RED_BITS, mode->redBits);
    glfwWindowHint(GLFW_GREEN_BITS, mode->greenBits);
    glfwWindowHint(GLFW_BLUE_BITS, mode->blueBits);
    glfwWindowHint(GLFW_REFRESH_RATE, mode->refreshRate);
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_AUTO_ICONIFY, GL_FALSE);

    // Create the new Window
    window = glfwCreateWindow(mode->width, mode->height, "Project Koi", monitor, NULL);
    DEBUG("CONTEXT - Window Created");

    return window;
}

void getGLFWInstanceExtensions(std::vector<const char *> & extensions)
{
    // Get required window extensions for vulkan
    uint32_t count;
    const char ** temp = glfwGetRequiredInstanceExtensions(&count);
    extensions.insert(extensions.end(), &temp[0], &temp[count]);
}

// ===============================================================================================================
//                                        DesktopContext Implementation
// ===============================================================================================================

DesktopContext::DesktopContext()
{
    // ====== Setup Window ======
    int result = glfwInit();
    VALIDATE(result == GLFW_TRUE, "GLFW Failed to Initialize");

    window = createBorderlessGLFWwindow();
    getGLFWInstanceExtensions(instanceExtensions);

    // ===== Setup Instance =====

    VALIDATE(supportsLayers(layers), "Requested Layers not supported");
    instance = createVkInstance(layers, instanceExtensions);

    this->createValidationDebugCallback();

    // ===== Setup Drawing Surface =====
    result = glfwCreateWindowSurface(instance, window, nullptr, &surface);
    VALIDATE(result == VK_SUCCESS, "Failed to create Surface %d", result);

    // ===== Setup VkPhysicalDevice =====
    physicalDevice = chooseVkPhysicalDevice(instance, surface, VK_PHYSICAL_DEVICE_FEATURES_NONE, deviceExtensions, queueFlags);

    std::vector<uint32_t> queueLocations;
    selectVkQueues(physicalDevice, this->queueFlags, this->queues, queueLocations, surface, 0);

    this->primaryGraphicsQueue = &this->queues[queueLocations[0]];
    this->primaryTransferQueue = &this->queues[queueLocations[1]];
    this->primaryPresentQueue = &this->queues[queueLocations[0]];

    DEBUG("CONTEXT - Found Graphics Queue - index: %d flags: %d", this->primaryGraphicsQueue->index, this->primaryGraphicsQueue->flags);
    DEBUG("CONTEXT - Found Transfer Queue - index: %d flags: %d", this->primaryTransferQueue->index, this->primaryTransferQueue->flags);
    DEBUG("CONTEXT - Found Present Queue - index: %d flags: %d", this->primaryPresentQueue->index, this->primaryPresentQueue->flags);

    for (auto& queue : queues)
        queueIndices.push_back(queue.index);

    // ===== Setup VkDevice =====
    device = createVkDevice(physicalDevice, getPhysicalDeviceFeatures(physicalDevice), layers, deviceExtensions, queues);

    // ===== Setup VkQueues =====
    for (auto& queue : queues)
        vkGetDeviceQueue(device, queue.index, 0, &queue.queue);

    // ===== Setup VkCommandPools =====
    for (auto& queue : queues)
    {
        VkCommandPoolCreateInfo poolInfo = queue.getVkCommandPoolCreateInfo();
        vkCreateCommandPool(device, &poolInfo, nullptr, &queue.commandPool);
    }

    DEBUG("CONTEXT - Context Created");
}

DesktopContext::~DesktopContext()
{
    for (auto& queue : queues)
    {
        vkDestroyCommandPool(device, queue.commandPool, nullptr);
    }

    vkDestroyDevice(device, nullptr);
    vkDestroySurfaceKHR(instance, surface, nullptr);
    this->destroyValidationDebugCallback();
    vkDestroyInstance(instance, nullptr);
    glfwDestroyWindow(window);
    glfwTerminate();

    DEBUG("CONTEXT - Context Destroyed");
}

void DesktopContext::init()
{
	app->sendMessageNow(GLFWwindowCreated, (void *) window);
}

void DesktopContext::update(long elapsedTime)
{
    glfwPollEvents();
	if (glfwWindowShouldClose(window))
	{
        app->sendMessage(Exit);
	}
}