#ifndef OVR_CONTEXT_H
#define OVR_CONTEXT_H

#include <render/Context.h>

#include <android_native_app_glue.h>

#include <VrApi.h>
#include <VrApi_Vulkan.h>
#include <VrApi_Helpers.h>

void getOVRInstanceExtensions(std::vector<const char *> & instanceExtensions);
void getOVRDeviceExtensions(std::vector<const char *> & deviceExtensions);

class OVRContext : public Context
{
    public:
    android_app * android_context;

    ovrJava java;
	ovrMobile * ovr;

	bool enteredVrMode = false;

    std::vector<const char *> layers = {"VK_LAYER_KHRONOS_validation"};
    std::vector<const char *> instanceExtensions = {"VK_EXT_debug_utils"};
    std::vector<const char *> deviceExtensions = {"VK_KHR_multiview"};
    std::vector<VkQueueFlagBits> queueFlags = {VK_QUEUE_GRAPHICS_BIT};

    OVRContext(struct android_app * android_context);
    ~OVRContext();

    void init();
    void update(long elapsedTime);
};

#endif