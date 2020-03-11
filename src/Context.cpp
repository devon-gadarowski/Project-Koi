#include <cstring>

#include <render/Context.h>

// ===============================================================================================================
//                                            VkInstance Helpers
// ===============================================================================================================

bool supportsLayers(const std::vector<const char *> & layers)
{
    uint32_t count;
    vkEnumerateInstanceLayerProperties(&count, nullptr);
    std::vector<VkLayerProperties> availableLayers(count);
    vkEnumerateInstanceLayerProperties(&count, availableLayers.data());

    for (auto & layer : layers)
    {
        bool found = false;
        for (auto & availableLayer : availableLayers)
        {
            if (strcmp(layer, availableLayer.layerName) == 0)
            {
                found = true;
            }
        }

        if (!found)
        {
            return false;
        }
    }

    return true;
}

void Context::createValidationDebugCallback()
{
    VkDebugUtilsMessengerCreateInfoEXT createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
    createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
    createInfo.pfnUserCallback = Context::debugCallback;
    createInfo.pUserData = nullptr;

    auto vkCreateDebugUtilsMessengerEXT = (PFN_vkCreateDebugUtilsMessengerEXT) vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
    VALIDATE(vkCreateDebugUtilsMessengerEXT != nullptr, "Failed to setup validation debug callbacks");

    vkCreateDebugUtilsMessengerEXT(this->instance, &createInfo, nullptr, &this->messanger);
}

void Context::destroyValidationDebugCallback()
{
    auto vkDestroyDebugUtilsMessengerEXT = (PFN_vkDestroyDebugUtilsMessengerEXT) vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
    VALIDATE(vkDestroyDebugUtilsMessengerEXT != nullptr, "Failed to destroy validation debug callbacks");

    vkDestroyDebugUtilsMessengerEXT(this->instance, this->messanger, nullptr);
}

VKAPI_ATTR VkBool32 VKAPI_CALL Context::debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData)
{
    if (VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT)
    {
        DEBUG("VALIDATION - %s", pCallbackData->pMessage);
    }
    else if (VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT)
    {
        DEBUG("VALIDATION - %s", pCallbackData->pMessage);
    }
    else if (VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT)
    {
        WARN("VALIDATION - %s", pCallbackData->pMessage);
    }
    else if (VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT)
    {
        PANIC("VALIDATION - %s", pCallbackData->pMessage);
    }

    return VK_TRUE;
}

VkInstance createVkInstance(const std::vector<const char *> & layers, const std::vector<const char *> & extensions)
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
    createInfo.enabledLayerCount = layers.size();
    createInfo.ppEnabledLayerNames = layers.data();
    createInfo.enabledExtensionCount = extensions.size();
    createInfo.ppEnabledExtensionNames = extensions.data();

    VkInstance instance;
    int result = vkCreateInstance(&createInfo, nullptr, &instance);
    VALIDATE(result == VK_SUCCESS, "Failed to create VkInstance");

    DEBUG("CONTEXT - VkInstance Created");
    return instance;
}

// ===============================================================================================================
//                                         VkPhysicalDevice Helpers
// ===============================================================================================================

VkPhysicalDevice chooseVkPhysicalDevice(VkInstance instance, VkSurfaceKHR surface, PhysicalDeviceFeaturesFlags features, const std::vector<const char *> & extensions, const std::vector<VkQueueFlagBits> & queues)
{
    VkPhysicalDevice physicalDevice;

    uint32_t deviceCount = 0;
    vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);
    std::vector<VkPhysicalDevice> physicalDevices(deviceCount);
    vkEnumeratePhysicalDevices(instance, &deviceCount, physicalDevices.data());

    for (auto & maybePhysicalDevice : physicalDevices)
    {
        if (!supportsFeatures(maybePhysicalDevice, features))
            continue;

        if (!supportsExtensions(maybePhysicalDevice, extensions))
            continue;

        if (!supportsQueues(maybePhysicalDevice, queues))
            continue;

        if (!supportsSurfaceFormats(maybePhysicalDevice, surface))
            continue;

        physicalDevice = maybePhysicalDevice;
        break; 
    }

    VALIDATE(physicalDevice != VK_NULL_HANDLE, "Failed to find suitable GPU");

    VkPhysicalDeviceProperties properties;
    vkGetPhysicalDeviceProperties(physicalDevice, &properties);
    DEBUG("CONTEXT - VkPhysicalDevice Name: %s", properties.deviceName);

    return physicalDevice;
}

void selectVkQueues(VkPhysicalDevice physicalDevice, const std::vector<VkQueueFlagBits> & queueFlags, std::vector<Queue> & queues, std::vector<uint32_t> & queueLocations, VkSurfaceKHR surface, int presentQueue)
{
    // this->queues --> vkGetDeviceQueues() (or something)
    std::vector<VkQueueFamilyProperties> queueProps;
    getQueueFamilyProperties(physicalDevice, queueProps);

    int map[queueProps.size()];

    for (int i = 0; i < queueProps.size(); i++)
        map[i] = -1;

    for (uint32_t i = 0; i < queueFlags.size(); i++)
    {
        bool found = false;

        // Look for exact match (dedicated queue)
        for (uint32_t j = 0; j < queueProps.size(); j++)
        {
            if (surface != VK_NULL_HANDLE && i == presentQueue)
            {
                VkBool32 presentSupport = VK_FALSE;
                vkGetPhysicalDeviceSurfaceSupportKHR(physicalDevice, j, surface, &presentSupport);
                
                if (presentSupport != VK_TRUE)
                {
                    continue;
                }
                else if (queueFlags[i] == VK_QUEUE_FLAG_BITS_MAX_ENUM)
                {
                    // Check if we have already selected this Queue
                    if (map[j] != -1)
                    {
                        continue;
                    }

                    // Mark Queue as taken (so we don't get duplicate queue objects)
                    map[j] = queues.size();

                    queueLocations.push_back(map[j]);
                    queues.push_back({queueProps[j].queueFlags, j, VK_NULL_HANDLE});
                    found = true;
                    break;
                }
            }

            if (queueProps[j].queueCount > 0 && (queueFlags[i] ^ queueProps[j].queueFlags) == 0)
            {
                // Check if we have already selected this Queue
                if (map[j] != -1)
                {
                    continue;
                }

                // Mark Queue as taken (so we don't get duplicate queue objects)
                map[j] = queues.size();

                queueLocations.push_back(map[j]);
                queues.push_back({queueProps[j].queueFlags, j, VK_NULL_HANDLE});
                found = true;
                break;
            }
        }

        if (found)
            continue;

        // Look for "close enough" match (generalized queue)
        for (uint32_t j = 0; j < queueProps.size(); j++)
        {
            if (surface != VK_NULL_HANDLE && i == presentQueue)
            {
                VkBool32 presentSupport = VK_FALSE;
                vkGetPhysicalDeviceSurfaceSupportKHR(physicalDevice, j, surface, &presentSupport);
                
                if (presentSupport != VK_TRUE)
                {
                    continue;
                }
                else if (queueFlags[i] == VK_QUEUE_FLAG_BITS_MAX_ENUM)
                {
                    // Check if we have already selected this Queue
                    if (map[j] != -1)
                    {
                        continue;
                    }

                    // Mark Queue as taken (so we don't get duplicate queue objects)
                    map[j] = queues.size();

                    queueLocations.push_back(map[j]);
                    queues.push_back({queueProps[j].queueFlags, j, VK_NULL_HANDLE});
                    found = true;
                    break;
                }
            }

            if (queueProps[j].queueCount > 0 && (queueFlags[i] & queueProps[j].queueFlags) == queueFlags[i])
            {
                // Check if we have already selected this Queue
                if (map[j] != -1)
                {
                    continue;
                }

                // Mark Queue as taken (so we don't get duplicate queue objects)
                map[j] = queues.size();

                queueLocations.push_back(map[j]);
                queues.push_back({queueProps[j].queueFlags, j, VK_NULL_HANDLE});
                found = true;
                break;
            }
        }

        if (found)
            continue;

        // Last resort -> try to map two requested queues to one
        for (uint32_t j = 0; j < queueProps.size(); j++)
        {
            if (surface != VK_NULL_HANDLE && i == presentQueue)
            {
                VkBool32 presentSupport = VK_FALSE;
                vkGetPhysicalDeviceSurfaceSupportKHR(physicalDevice, j, surface, &presentSupport);
                
                if (presentSupport != VK_TRUE)
                {
                    continue;
                }
                else if (queueFlags[i] == VK_QUEUE_FLAG_BITS_MAX_ENUM)
                {
                    // Check if we have already selected this Queue
                    if (map[j] != -1)
                    {
                        queueLocations.push_back(map[j]);
                        found = true;
                        break;
                    }
                }
            }

            if (queueProps[j].queueCount > 0 && (queueFlags[i] & queueProps[j].queueFlags) == queueFlags[i])
            {
                // Check if we have already selected this Queue
                if (map[j] != -1)
                {
                    queueLocations.push_back(map[j]);
                    found = true;
                    break;
                }
            }
        }

        VALIDATE(found, "Failed to select a queue");
    }
}

VkPhysicalDeviceFeatures getPhysicalDeviceFeatures(VkPhysicalDevice physicalDevice)
{
    VkPhysicalDeviceFeatures features;
    vkGetPhysicalDeviceFeatures(physicalDevice, &features);
    return features;
}

void getSurfaceFormats(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface, std::vector<VkSurfaceFormatKHR> & formats)
{
    uint32_t count = 0;
    vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &count, nullptr);
    formats.resize(count);
    vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &count, formats.data());  
}

void getExtensionProperties(VkPhysicalDevice physicalDevice, std::vector<VkExtensionProperties> & extensionProperties)
{
    uint32_t count = 0;
    vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &count, nullptr);
    extensionProperties.resize(count);
    vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &count, extensionProperties.data()); 
}

void getQueueFamilyProperties(VkPhysicalDevice physicalDevice, std::vector<VkQueueFamilyProperties> & queueProperties)
{
    uint32_t count = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &count, nullptr);
    queueProperties.resize(count);
    vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &count, queueProperties.data()); 
}

bool supportsFeatures(VkPhysicalDevice physicalDevice, PhysicalDeviceFeaturesFlags features)
{
    if (features == 0)
        return true;

    VkPhysicalDeviceFeatures supportedFeatures;
    vkGetPhysicalDeviceFeatures(physicalDevice, &supportedFeatures);

    // TODO: Check each feature as necessary here

    return true;
}

bool supportsExtensions(VkPhysicalDevice physicalDevice, const std::vector<const char *> & extensions)
{
    if (extensions.size() == 0)
        return true;

    std::vector<VkExtensionProperties> extensionProperties;
    getExtensionProperties(physicalDevice, extensionProperties);

    for (auto & extension : extensions)
    {
        bool found = false;
        for (auto& supportedExtension : extensionProperties)
        {
            if (strcmp(supportedExtension.extensionName, extension) == 0)
            {
                found = true;
            }
        }

        if (!found)
        {
            return false;
        }
    }

    return true;
}

bool supportsQueues(VkPhysicalDevice physicalDevice, const std::vector<VkQueueFlagBits> & queues)
{
    if (queues.size() == 0)
        return true;

    std::vector<VkQueueFamilyProperties> queueProperties;
    getQueueFamilyProperties(physicalDevice, queueProperties);

    for (auto & queue : queues)
    {
        bool found = false;
        for (auto & supportedQueue : queueProperties)
        {
            if (supportedQueue.queueCount > 0 && (queue & supportedQueue.queueFlags) == queue)
            {
                found = true;
                break;
            }
        }

        if (!found)
        {
            return false;
        }
    }

    return true;
}

bool supportsSurfaceFormats(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface)
{
    if (surface == VK_NULL_HANDLE)
        return true;

    std::vector<VkSurfaceFormatKHR> formats;
    getSurfaceFormats(physicalDevice, surface, formats);

    return formats.size() > 0;
}

// ===============================================================================================================
//                                            VkDevice Helpers
// ===============================================================================================================

VkDevice createVkDevice(VkPhysicalDevice physicalDevice, VkPhysicalDeviceFeatures deviceFeatures, std::vector<const char *> layers, std::vector<const char *> extensions, std::vector<Queue> & queues)
{
    VkDevice device;
    std::vector<VkDeviceQueueCreateInfo> queueInfos(queues.size());

    float priority = 1.0f;
    for (int i = 0; i < queueInfos.size(); i++)
    {
        queueInfos[i].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queueInfos[i].pNext = nullptr;
        queueInfos[i].flags = 0;
        queueInfos[i].queueFamilyIndex = queues[i].index;
        queueInfos[i].queueCount = 1;
        queueInfos[i].pQueuePriorities = &priority;
    }

    VkDeviceCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    createInfo.pNext = nullptr;
    createInfo.flags = 0;
    createInfo.queueCreateInfoCount = queueInfos.size();
    createInfo.pQueueCreateInfos = queueInfos.data();
    createInfo.enabledLayerCount = layers.size();
    createInfo.ppEnabledLayerNames = layers.data();
    createInfo.enabledExtensionCount = extensions.size();
    createInfo.ppEnabledExtensionNames = extensions.data();
    createInfo.pEnabledFeatures = &deviceFeatures;

    int result = vkCreateDevice(physicalDevice, &createInfo, nullptr, &device);
    VALIDATE(result == VK_SUCCESS, "Failed to create VkDevice %d", result);

    DEBUG("CONTEXT - VkDevice Created");
    return device;
}