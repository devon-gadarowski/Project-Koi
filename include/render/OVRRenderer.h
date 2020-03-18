#ifndef OVR_RENDERER_H
#define OVR_RENDERER_H

#include <render/OVRContext.h>
#include <render/Renderer.h>
#include <render/Utilities.h>

#include <android_native_app_glue.h>
#include <VrApi.h>

class OVRRenderer : public Renderer
{
    public:
    OVRContext * context;

    ovrTextureSwapChain * swapchain;

    OVRRenderer(OVRContext * context);
    ~OVRRenderer();

    VkCommandBuffer getNextCommandBuffer();
    void render(VkCommandBuffer commandBuffer);
    void present();

    void init();
    void update(long elapsedTime);
};

#endif