#ifndef RENDERER_H
#define RENDERER_H

#include <system/System.h>
#include <render/Context.h>

class Renderer : public System
{
    public:
    const VkSampleCountFlagBits sample_count = VK_SAMPLE_COUNT_4_BIT;

    Context * context;

    uint32_t currentImageIndex = 0;
    uint32_t frameCount = 0;

    uint32_t length;
    VkExtent2D extent;

    std::vector<VkImage> images;
    std::vector<VkImageView> imageviews;
    std::vector<VkCommandBuffer> commandbuffers;
    std::vector<VkFence> fences;

    VkFormat colorFormat;
    VkImage colorImage;
    VkDeviceMemory colorImageMemory;
    VkImageView colorImageView;

    VkFormat depthFormat;
    VkImage depthImage;
    VkDeviceMemory depthImageMemory;
    VkImageView depthImageView;;

    VkSemaphore imageAvailable;
    VkSemaphore renderFinished;

    Renderer() {}
    virtual ~Renderer() {}

    virtual VkViewport getDefaultVkViewport();
    virtual VkRect2D getDefaultScissor();
    virtual VkPipelineRasterizationStateCreateInfo getDefaultRasterizer();
    virtual VkPipelineMultisampleStateCreateInfo getDefaultMultisampling();
    virtual VkPipelineDepthStencilStateCreateInfo getDefaultDepthStencil();
    virtual void getDefaultColorBlendAttachments(std::vector<VkPipelineColorBlendAttachmentState> & colorBlendAttachments);
    virtual VkPipelineColorBlendStateCreateInfo getDefaultColorBlend(std::vector<VkPipelineColorBlendAttachmentState> & colorBlendAttachments);

    virtual VkCommandBuffer getNextCommandBuffer() = 0;
    virtual void render(VkCommandBuffer commandBuffer) = 0;
    virtual void present() = 0;
};

#endif