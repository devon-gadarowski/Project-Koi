#ifndef CONTEXT_H
#define CONTEXT_H

#include <vector>

#include<vulkan/vulkan.h>

#include <system/Log.h>
#include <system/System.h>

#define APPLICATION_NAME "Project Koi"
#define APPLICATION_VERSION 1
#define ENGINE_NAME "Koi Engine"
#define ENGINE_VERSION 4

// ===============================================================================================================
//                                             Queue Container
// ===============================================================================================================

struct Queue
{
	VkQueueFlags flags;
	uint32_t index;
	VkQueue queue;
	VkCommandPool commandPool;

	VkCommandPoolCreateInfo getVkCommandPoolCreateInfo()
	{
		VkCommandPoolCreateInfo createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		createInfo.pNext = nullptr;
		createInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
		createInfo.queueFamilyIndex = this->index;

		return createInfo;
	}
};

// ===============================================================================================================
//                                            VkInstance Helpers
// ===============================================================================================================

bool supportsLayers(const std::vector<const char *> & layers);
VkInstance createVkInstance(const std::vector<const char *> & layers, const std::vector<const char *> & extensions);

// ===============================================================================================================
//                                         VkPhysicalDevice Helpers
// ===============================================================================================================

typedef uint64_t PhysicalDeviceFeaturesFlags;

enum PhysicalDeviceFeatures : uint64_t
{
    VK_PHYSICAL_DEVICE_FEATURES_NONE = 0,
};

VkPhysicalDevice chooseVkPhysicalDevice(VkInstance instance, VkSurfaceKHR surface, PhysicalDeviceFeaturesFlags features, const std::vector<const char *> & extensions, const std::vector<VkQueueFlagBits> & queues);
void selectVkQueues(VkPhysicalDevice physicalDevice, const std::vector<VkQueueFlagBits> & queueFlags, std::vector<Queue> & queues, std::vector<uint32_t> & queueLocations, VkSurfaceKHR surface, int presentQueue);
VkPhysicalDeviceFeatures getPhysicalDeviceFeatures(VkPhysicalDevice physicalDevice);
void getSurfaceFormats(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface, std::vector<VkSurfaceFormatKHR> & formats);
void getExtensionProperties(VkPhysicalDevice physicalDevice, std::vector<VkExtensionProperties> & extensionProperties);
void getQueueFamilyProperties(VkPhysicalDevice physicalDevice, std::vector<VkQueueFamilyProperties> & queueProperties);
bool supportsFeatures(VkPhysicalDevice physicalDevice, PhysicalDeviceFeaturesFlags features);
bool supportsExtensions(VkPhysicalDevice physicalDevice, const std::vector<const char *> & extensions);
bool supportsQueues(VkPhysicalDevice physicalDevice, const std::vector<VkQueueFlagBits> & queues);
bool supportsSurfaceFormats(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface);

// ===============================================================================================================
//                                            VkDevice Helpers
// ===============================================================================================================

VkDevice createVkDevice(VkPhysicalDevice physicalDevice, VkPhysicalDeviceFeatures deviceFeatures, std::vector<const char *> layers, std::vector<const char *> extensions, std::vector<Queue> & queues);

// ===============================================================================================================
//                                             Context Interface
// ===============================================================================================================

class Context : public System
{
	public:
    VkInstance instance;
    VkPhysicalDevice physicalDevice;
    VkDevice device;

	Queue * primaryGraphicsQueue;
	Queue * primaryTransferQueue;
	Queue * primaryPresentQueue;

	std::vector<uint32_t> queueIndices;
    std::vector<Queue> queues;

    std::vector<const char *> layers = {};
    std::vector<const char *> instanceExtensions = {};
    std::vector<const char *> deviceExtensions = {};
    std::vector<VkQueueFlagBits> queueFlags = {};

    Context() {}
    virtual ~Context() {}

	VkDebugUtilsMessengerEXT messanger;
	void createValidationDebugCallback();
	void destroyValidationDebugCallback();
	static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData);
};

#endif