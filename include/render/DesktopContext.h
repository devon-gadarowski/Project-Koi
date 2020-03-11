#ifndef DESKTOP_CONTEXT_H
#define DESKTOP_CONTEXT_H

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <render/Context.h>

GLFWwindow * createBorderlessGLFWwindow();
void getGLFWInstanceExtensions(std::vector<const char *> & extensions);

class DesktopContext : public Context
{
    public:
    GLFWwindow * window;
    VkSurfaceKHR surface;

    std::vector<const char *> layers = {"VK_LAYER_KHRONOS_validation", "VK_LAYER_LUNARG_standard_validation"};
    std::vector<const char *> instanceExtensions = {"VK_EXT_debug_utils"};
    std::vector<const char *> deviceExtensions = {"VK_KHR_swapchain"};
    std::vector<VkQueueFlagBits> queueFlags = {VK_QUEUE_GRAPHICS_BIT, VK_QUEUE_TRANSFER_BIT};

    DesktopContext();
    ~DesktopContext();

    void init();
    void update(long elapsedTime);
};

#endif