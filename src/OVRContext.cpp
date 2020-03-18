#include <render/OVRContext.h>

#include <vulkan_wrapper.h>

void parseVrApiExtensionsList(char * extensionNames, const char * extensions[], uint32_t * extensionsCount)
{
	char * extensionIterator = extensionNames;
	int i = 0;

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
}

void getOVRInstanceExtensions(std::vector<const char *> & instanceExtensions)
{
	char extensionNames[4096];
	uint32_t extensionsBufferSize = sizeof(extensionNames);
	int result = vrapi_GetInstanceExtensionsVulkan(extensionNames, &extensionsBufferSize);
    VALIDATE(result == ovrSuccess, "Failed to get VrApi Instance Extensions %d", result);

	const char * extensions[64];
	uint32_t extensionCount;
    parseVrApiExtensionsList(extensionNames, extensions, &extensionCount);

    for (int i = 0; i < extensionCount; i++)
        instanceExtensions.push_back(extensions[i]);
}

void getOVRDeviceExtensions(std::vector<const char *> & deviceExtensions)
{
	char extensionNames[4096];
	uint32_t extensionsBufferSize = sizeof(extensionNames);
	int result = vrapi_GetDeviceExtensionsVulkan(extensionNames, &extensionsBufferSize);
    VALIDATE(result == ovrSuccess, "Failed to get VrApi Instance Extensions %d", result);

	const char * extensions[64];
	uint32_t extensionCount;
    parseVrApiExtensionsList(extensionNames, extensions, &extensionCount);

    for (int i = 0; i < extensionCount; i++)
        deviceExtensions.push_back(extensions[i]);
}

void OVRContext::init()
{

}

void OVRContext::update(long elapsedTime)
{
    while (true)
    {
        int events;
        struct android_poll_source * source;
        const int timeoutMilliseconds = (this->ovr == nullptr && android_context->destroyRequested == 0) ? -1 : 0;
        if (ALooper_pollAll(timeoutMilliseconds, NULL, &events, (void **)&source ) < 0)
        {
            //DEBUG("--Done.");
            break;
        }

        // Process this event.
        if (source != nullptr)
        {
            source->process(android_context, source);
        }

        // Handle VrMode changes
        if (!this->enteredVrMode && android_context->activityState == APP_CMD_RESUME && !(android_context->destroyRequested) && android_context->window != NULL)
        {
            INFO("Entering Vr Mode...");
            ovrModeParmsVulkan parms = vrapi_DefaultModeParmsVulkan(&this->java, (unsigned long long) this->primaryGraphicsQueue->queue);
            parms.ModeParms.Flags &= ~VRAPI_MODE_FLAG_RESET_WINDOW_FULLSCREEN;
            parms.ModeParms.Flags = VRAPI_MODE_FLAG_NATIVE_WINDOW;
            parms.ModeParms.WindowSurface = (size_t)android_context->window;
            parms.ModeParms.Display = 0;
            parms.ModeParms.ShareContext = 0;

            this->ovr = vrapi_EnterVrMode((ovrModeParms *) &parms);
            this->enteredVrMode = true;
            INFO("--Done.");
        }
        else if (this->enteredVrMode && (android_context->activityState != APP_CMD_RESUME || android_context->destroyRequested || android_context->window == NULL))
        {
            INFO("Leaving Vr Mode...");
            vrapi_LeaveVrMode(this->ovr);
            this->enteredVrMode = false;
            INFO("--Done.");
        }
    }
}

OVRContext::OVRContext(struct android_app * android_context)
{
    InitVulkan();

    this->android_context = android_context;

    // ===== Initialive OVR API =====

    this->java.ActivityObject = android_context->activity->clazz;
    this->java.Vm = android_context->activity->vm;
    this->java.Vm->AttachCurrentThread(&this->java.Env, nullptr);

	ovrInitParms initParms = vrapi_DefaultInitParms(&this->java);
	initParms.GraphicsAPI = VRAPI_GRAPHICS_API_VULKAN_1;

    int result = vrapi_Initialize(&initParms);
    VALIDATE(result == VRAPI_INITIALIZE_SUCCESS, "Failed to initialized VrApi %d", result);

    // ===== Setup Instance =====

    getOVRInstanceExtensions(instanceExtensions);

    VALIDATE(supportsLayers(layers), "Requested Layers not supported");
    instance = createVkInstance(layers, instanceExtensions);

    this->createValidationDebugCallback();

    // ===== Setup VkPhysicalDevice =====

    getOVRDeviceExtensions(deviceExtensions);
    physicalDevice = chooseVkPhysicalDevice(instance, VK_NULL_HANDLE, VK_PHYSICAL_DEVICE_FEATURES_NONE, deviceExtensions, queueFlags);

    std::vector<uint32_t> queueLocations;
    selectVkQueues(physicalDevice, this->queueFlags, this->queues, queueLocations, VK_NULL_HANDLE, 0);

    this->primaryGraphicsQueue = &this->queues[queueLocations[0]];
    this->primaryTransferQueue = &this->queues[queueLocations[0]];
    this->primaryPresentQueue = &this->queues[queueLocations[0]];

    DEBUG("CONTEXT - Found Graphics Queue - index: %d flags: %d", this->primaryGraphicsQueue->index, this->primaryGraphicsQueue->flags);
    DEBUG("CONTEXT - Found Present Queue - index: %d flags: %d", this->primaryPresentQueue->index, this->primaryPresentQueue->flags);

    for (auto& queue : queues)
        queueIndices.push_back(queue.index);

    // ===== Setup VkDevice =====

    device = createVkDevice(physicalDevice, getPhysicalDeviceFeatures(physicalDevice), layers, deviceExtensions, queues);

    // ===== Setup VrApi System Vulkan =====

    ovrSystemCreateInfoVulkan systemInfo = {};
    systemInfo.Instance = this->instance;
    systemInfo.PhysicalDevice = this->physicalDevice;
    systemInfo.Device = this->device;

    result = vrapi_CreateSystemVulkan(&systemInfo);
    VALIDATE(result == ovrSuccess, "Failed to create VrApi System %d", result);

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

OVRContext::~OVRContext()
{
    vrapi_DestroySystemVulkan();

    for (auto& queue : queues)
    {
        vkDestroyCommandPool(device, queue.commandPool, nullptr);
    }

    vkDestroyDevice(device, nullptr);
    this->destroyValidationDebugCallback();
    vkDestroyInstance(instance, nullptr);

    DEBUG("CONTEXT - Context Destroyed");
}