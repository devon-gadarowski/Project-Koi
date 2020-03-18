#ifndef SCENE_H
#define SCENE_H

#include <render/KoiVulkan.h>

#include <system/System.h>
#include <render/Context.h>
#include <render/Renderer.h>

struct UniformBuffer
{
    void * data;
    VkBuffer buffer;
    VkDeviceMemory memory;
};

class Scene : public System
{
    public:
    Context * context;
    Renderer * renderer;

    VkRenderPass renderPass;
    std::vector<VkFramebuffer> framebuffers;

    VkDescriptorPool descriptorPool;
    VkDescriptorSetLayout descriptorSetLayout;
	std::vector<VkDescriptorSet> descriptorSets;

    Scene() {};
    ~Scene() {};

    virtual void draw(VkCommandBuffer commandBuffer) = 0;
};

#endif