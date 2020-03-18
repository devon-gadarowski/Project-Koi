
#include <vector>

#include <vulkan/vulkan.h>

#include <system/Log.h>
#include <render/DesktopRenderer.h>
#include <render/Utilities.h>

void getVkSurfacePresentModes(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface, std::vector<VkPresentModeKHR> & presentModes);
void getVkSurfaceFormats(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface, std::vector<VkSurfaceFormatKHR> & surfaceFormats);
uint32_t chooseLength(VkSurfaceCapabilitiesKHR & capabilities);
VkSurfaceFormatKHR chooseSurfaceFormat(std::vector<VkSurfaceFormatKHR> & surfaceFormats);
VkPresentModeKHR choosePresentMode(std::vector<VkPresentModeKHR> & presentModes);

void DesktopRenderer::init()
{

}

void DesktopRenderer::update(long elapsedTime)
{

}

VkCommandBuffer DesktopRenderer::getNextCommandBuffer()
{
    vkAcquireNextImageKHR(context->device, swapchain, UINT64_MAX, imageAvailable, VK_NULL_HANDLE, &currentImageIndex);

	vkWaitForFences(context->device, 1, &fences[currentImageIndex], VK_TRUE, std::numeric_limits<uint64_t>::max());
	vkResetFences(context->device, 1, &fences[currentImageIndex]);

    VkCommandBufferBeginInfo beginInfo = {};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = 0;
    beginInfo.pInheritanceInfo = nullptr;

    int result = vkBeginCommandBuffer(commandbuffers[currentImageIndex], &beginInfo);
    VALIDATE(result == VK_SUCCESS, "RENDERER - Failed to begin recording command buffer! %d", result);

    VkImageMemoryBarrier barrier;
    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.pNext = nullptr;
    barrier.srcAccessMask = 0;
    barrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    barrier.newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.image = images[currentImageIndex];
    barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    barrier.subresourceRange.baseMipLevel = 0;
    barrier.subresourceRange.levelCount = 1;
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.layerCount = 1;

    vkCmdPipelineBarrier(commandbuffers[currentImageIndex], VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
                         VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, 0, 0, nullptr, 0, nullptr,
                         1, &barrier);

    return commandbuffers[currentImageIndex];
}

void DesktopRenderer::render(VkCommandBuffer commandbuffer)
{
    VkImageMemoryBarrier barrier;
    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.pNext = nullptr;
    barrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    barrier.dstAccessMask = 0;
    barrier.oldLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    barrier.newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.image = images[currentImageIndex];
    barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    barrier.subresourceRange.baseMipLevel = 0;
    barrier.subresourceRange.levelCount = 1;
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.layerCount = 1;

    vkCmdPipelineBarrier(commandbuffers[currentImageIndex], VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
                         VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, 0, 0, nullptr, 0, nullptr,
                         1, &barrier);

    vkEndCommandBuffer(commandbuffer);

	VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};

	VkSubmitInfo submitInfo = {};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.waitSemaphoreCount = 1;
	submitInfo.pWaitSemaphores = &imageAvailable;
	submitInfo.pWaitDstStageMask = waitStages;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &commandbuffer;
	submitInfo.signalSemaphoreCount = 1;
	submitInfo.pSignalSemaphores = &renderFinished;

	VkResult result = vkQueueSubmit(context->primaryGraphicsQueue->queue, 1, &submitInfo, fences[currentImageIndex]);
	VALIDATE(result == VK_SUCCESS, "RENDERER - Failed to submit draw command buffer %d", result);
}

void DesktopRenderer::present()
{
	VkResult result = VK_SUCCESS;

	VkPresentInfoKHR presentInfo = {};
	presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	presentInfo.pNext = nullptr;
	presentInfo.waitSemaphoreCount = 1;
	presentInfo.pWaitSemaphores = &renderFinished;
	presentInfo.swapchainCount = 1;
	presentInfo.pSwapchains = &swapchain;
	presentInfo.pImageIndices = &currentImageIndex;
	presentInfo.pResults = &result;

	vkQueuePresentKHR(context->primaryPresentQueue->queue, &presentInfo);
	VALIDATE(result == VK_SUCCESS, "RENDERER - Failed to present swapchain image %d", result);
}

DesktopRenderer::DesktopRenderer(DesktopContext * context)
{
    // ===== Initialization =====

    this->context = context;

    VkSurfaceCapabilitiesKHR capabilities;
    std::vector<VkPresentModeKHR> presentModes;
    std::vector<VkSurfaceFormatKHR> surfaceFormats;

    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(context->physicalDevice, context->surface, &capabilities);
    getVkSurfacePresentModes(context->physicalDevice, context->surface, presentModes);
    getVkSurfaceFormats(context->physicalDevice, context->surface, surfaceFormats);

    this->length = chooseLength(capabilities);
    this->extent = capabilities.currentExtent;

    this->colorFormat = chooseColorFormat(context->physicalDevice);
    this->depthFormat = chooseDepthFormat(context->physicalDevice);

    VkPresentModeKHR presentMode = choosePresentMode(presentModes);
    VkSurfaceFormatKHR surfaceFormat = chooseSurfaceFormat(surfaceFormats);

    // ===== VkSwapchainKHR =====

    VkSwapchainCreateInfoKHR swapchainInfo = {};
    swapchainInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    swapchainInfo.pNext = nullptr;
    swapchainInfo.flags = 0;
    swapchainInfo.surface = context->surface;
    swapchainInfo.minImageCount = this->length;
    swapchainInfo.imageFormat = surfaceFormat.format;
    swapchainInfo.imageColorSpace = surfaceFormat.colorSpace;
    swapchainInfo.imageExtent = capabilities.currentExtent;
    swapchainInfo.imageArrayLayers = 1;
    swapchainInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    swapchainInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    swapchainInfo.queueFamilyIndexCount = 1;
    swapchainInfo.pQueueFamilyIndices = &context->primaryGraphicsQueue->index;
    swapchainInfo.preTransform = capabilities.currentTransform;
    swapchainInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    swapchainInfo.presentMode = presentMode;
    swapchainInfo.clipped = VK_TRUE;
    swapchainInfo.oldSwapchain = VK_NULL_HANDLE;
    int result = vkCreateSwapchainKHR(context->device, &swapchainInfo, nullptr, &swapchain);
    VALIDATE(result == VK_SUCCESS, "Failed to create VkSwapchainKHR %d", result);

    vkGetSwapchainImagesKHR(context->device, swapchain, &length, nullptr);
    images.resize(length);
    vkGetSwapchainImagesKHR(context->device, swapchain, &length, images.data());

    imageviews.resize(length);
	for (int i = 0; i < length; i++)
	{
		createVkImageView(context->physicalDevice, context->device, images[i], VK_IMAGE_VIEW_TYPE_2D,
			              colorFormat, 1, 1, VK_IMAGE_ASPECT_COLOR_BIT, &imageviews[i]);
	}

    // ===== MSAA Color Resolve Image =====

    createVkImage(context, VK_IMAGE_TYPE_2D, colorFormat,
                    extent, 1, 1, sample_count, VK_IMAGE_TILING_OPTIMAL,
                    VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
                    VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &colorImage, &colorImageMemory);

    transitionImageLayout(context->device, context->primaryGraphicsQueue->queue, context->primaryGraphicsQueue->commandPool, colorImage, colorFormat, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, 1);

    createVkImageView(context->physicalDevice, context->device, colorImage, VK_IMAGE_VIEW_TYPE_2D,
                        colorFormat, 1, 1, VK_IMAGE_ASPECT_COLOR_BIT,
                        &colorImageView);

    // ===== Depth Image =====

    createVkImage(context, VK_IMAGE_TYPE_2D, depthFormat,
                    extent, 1, 1, sample_count, VK_IMAGE_TILING_OPTIMAL,
                    VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
                    VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &depthImage, &depthImageMemory);

    transitionImageLayout(context->device, context->primaryGraphicsQueue->queue, context->primaryGraphicsQueue->commandPool, depthImage, depthFormat, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, 1);

    createVkImageView(context->physicalDevice, context->device, depthImage, VK_IMAGE_VIEW_TYPE_2D,
                        depthFormat, 1, 1, VK_IMAGE_ASPECT_DEPTH_BIT, &depthImageView);

    // ===== Queue Synchronization Objects =====

	fences.resize(length);
	for (int i = 0; i < length; i++)
	{
        VkFenceCreateInfo fenceInfo = {};
        fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

		vkCreateFence(context->device, &fenceInfo, nullptr, &fences[i]);
	}

	VkSemaphoreCreateInfo semaphoreInfo = {};
	semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

	vkCreateSemaphore(context->device, &semaphoreInfo, nullptr, &imageAvailable);
	vkCreateSemaphore(context->device, &semaphoreInfo, nullptr, &renderFinished);

    // ===== Command Buffers =====

    commandbuffers.resize(this->length);

    VkCommandBufferAllocateInfo commandbufferInfo = {};
    commandbufferInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    commandbufferInfo.pNext = nullptr;
    commandbufferInfo.commandPool = context->primaryGraphicsQueue->commandPool;
    commandbufferInfo.commandBufferCount = commandbuffers.size();
    commandbufferInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;

    vkAllocateCommandBuffers(context->device, &commandbufferInfo, commandbuffers.data());

    DEBUG("RENDERER - Renderer Created");
}

DesktopRenderer::~DesktopRenderer()
{
	vkDestroySemaphore(context->device, imageAvailable, nullptr);
	vkDestroySemaphore(context->device, renderFinished, nullptr);

	for (int i = 0; i < length; i++)
	{
		vkDestroyFence(context->device, fences[i], nullptr);
		vkDestroyImageView(context->device, imageviews[i], nullptr);
	}

    vkDestroyImageView(context->device, colorImageView, nullptr);
    vkDestroyImage(context->device, colorImage, nullptr);
    vkFreeMemory(context->device, colorImageMemory, nullptr);

    vkDestroyImageView(context->device, depthImageView, nullptr);
    vkDestroyImage(context->device, depthImage, nullptr);
    vkFreeMemory(context->device, depthImageMemory, nullptr);

    vkDestroySwapchainKHR(context->device, swapchain, nullptr);

    DEBUG("RENDERER - Renderer Destroyed");
}

void getVkSurfacePresentModes(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface, std::vector<VkPresentModeKHR> & presentModes)
{
    uint32_t count = 0;
    vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, &count, nullptr);
    presentModes.resize(count);
    vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, &count, presentModes.data());
}

void getVkSurfaceFormats(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface, std::vector<VkSurfaceFormatKHR> & surfaceFormats)
{
    uint32_t count = 0;
    vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &count, nullptr);
    surfaceFormats.resize(count);
    vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &count, surfaceFormats.data());
}

uint32_t chooseLength(VkSurfaceCapabilitiesKHR & capabilities)
{
    if (capabilities.maxImageCount != 0)
        return std::min(capabilities.minImageCount + 1, capabilities.maxImageCount);
    else
        return capabilities.minImageCount + 1;
}

VkSurfaceFormatKHR chooseSurfaceFormat(std::vector<VkSurfaceFormatKHR> & surfaceFormats)
{
    VkSurfaceFormatKHR surfaceFormat;
    std::vector<VkFormat> colorCandidates = {VK_FORMAT_R8G8B8A8_UNORM, VK_FORMAT_B8G8R8A8_UNORM};

    surfaceFormat = surfaceFormats[0];
    for (auto& colorCandidate : colorCandidates)
    {
        bool found = false;
        for (int i = 0; i < surfaceFormats.size(); i++)
        {
            if (surfaceFormats[i].format == colorCandidate)
            {
                surfaceFormat = surfaceFormats[i];
                found = true;
                break;
            }
        }
        if (found)
        {
            break;
        }
    }

    VALIDATE(surfaceFormat.format != VK_FORMAT_UNDEFINED, "RENDER_FRAMEWORK - Failed to find valid color image format");

    return surfaceFormat;
}

VkPresentModeKHR choosePresentMode(std::vector<VkPresentModeKHR> & presentModes)
{
    VkPresentModeKHR presentMode;

    std::vector<VkPresentModeKHR> presentCandidates = {VK_PRESENT_MODE_MAILBOX_KHR, VK_PRESENT_MODE_FIFO_KHR};

    presentMode = VK_PRESENT_MODE_FIFO_KHR;
    for (auto& candidate : presentCandidates)
    {
        bool found = false;
        for (int i = 0; i < presentModes.size(); i++)
        {
            if (presentModes[i] == candidate)
            {
                presentMode = candidate;
                found = true;
                break;
            }
        }
        if (found)
        {
            break;
        }
    }

    return presentMode;
}