#ifndef DESKTOP_RENDERER_H
#define DESKTOP_RENDERER_H

#include <render/Renderer.h>
#include <render/DesktopContext.h>

class DesktopRenderer : public Renderer
{
    public:
    DesktopContext * context;

    VkSurfaceFormatKHR surfaceFormat;
    VkSwapchainKHR swapchain;

    DesktopRenderer(DesktopContext * context);
    ~DesktopRenderer();

    VkCommandBuffer getNextCommandBuffer();
    void render(VkCommandBuffer commandBuffer);
    void present();

    void init();
    void update(long elapsedTime);
};

#endif