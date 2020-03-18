#include <render/OVRRenderer.h>

void OVRRenderer::init()
{

}

void OVRRenderer::update(long elapsedTime)
{

}

VkCommandBuffer OVRRenderer::getNextCommandBuffer()
{
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
    barrier.subresourceRange.layerCount = 2;

    vkCmdPipelineBarrier(commandbuffers[currentImageIndex], VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
                         VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, 0, 0, nullptr, 0, nullptr,
                         1, &barrier);

    return commandbuffers[currentImageIndex];
}

void OVRRenderer::render(VkCommandBuffer commandbuffer)
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
    barrier.subresourceRange.layerCount = 2;

    vkCmdPipelineBarrier(commandbuffers[currentImageIndex], VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
                         VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, 0, 0, nullptr, 0, nullptr,
                         1, &barrier);

    vkEndCommandBuffer(commandbuffer);

	VkSubmitInfo submitInfo = {};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.waitSemaphoreCount = 0;
	submitInfo.pWaitSemaphores = nullptr;
	submitInfo.pWaitDstStageMask = nullptr;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &commandbuffer;
	submitInfo.signalSemaphoreCount = 0;
	submitInfo.pSignalSemaphores = nullptr;

	VkResult result = vkQueueSubmit(context->primaryGraphicsQueue->queue, 1, &submitInfo, fences[currentImageIndex]);
	VALIDATE(result == VK_SUCCESS, "RENDERER - Failed to submit draw command buffer %d", result);
}

void OVRRenderer::present()
{
	double currentTime = vrapi_GetTimeInSeconds();
	double predictedDisplayTime = vrapi_GetPredictedDisplayTime(context->ovr, this->frameCount);
	double elapsedTime = predictedDisplayTime - currentTime;
	const ovrTracking2 tracking = vrapi_GetPredictedTracking2(context->ovr, predictedDisplayTime);

	ovrLayerProjection2 layer = vrapi_DefaultLayerProjection2();

	layer.HeadPose = tracking.HeadPose;
	layer.Textures[0].ColorSwapChain = this->swapchain;
	layer.Textures[0].SwapChainIndex = this->currentImageIndex;
	layer.Textures[0].TexCoordsFromTanAngles = ovrMatrix4f_TanAngleMatrixFromProjection(&tracking.Eye[0].ProjectionMatrix);
	layer.Textures[1].ColorSwapChain = this->swapchain;
	layer.Textures[1].SwapChainIndex = this->currentImageIndex;
	layer.Textures[1].TexCoordsFromTanAngles = ovrMatrix4f_TanAngleMatrixFromProjection(&tracking.Eye[1].ProjectionMatrix);

	const ovrLayerHeader2 * layers[] =
	{
		&layer.Header
	};

	ovrSubmitFrameDescription2 frameDesc = {};
	frameDesc.Flags = 0;
	frameDesc.SwapInterval = 1;
	frameDesc.FrameIndex = this->frameCount;
	frameDesc.DisplayTime = predictedDisplayTime;
	frameDesc.LayerCount = 1;
	frameDesc.Layers = layers;

	// Hand over the eye images to the time warp.
	int result = vrapi_SubmitFrame2(context->ovr, &frameDesc);
    VALIDATE(result == ovrSuccess, "Failed to submit frame to vr api %d", result);

    this->currentImageIndex++;
    this->currentImageIndex %= this->length;
}

OVRRenderer::OVRRenderer(OVRContext * context)
{
    // ===== Initialization =====

    this->context = context;

    this->colorFormat = chooseColorFormat(context->physicalDevice);
    this->depthFormat = chooseDepthFormat(context->physicalDevice);

	this->extent.width = vrapi_GetSystemPropertyInt(&context->java, VRAPI_SYS_PROP_SUGGESTED_EYE_TEXTURE_WIDTH);
	this->extent.height = vrapi_GetSystemPropertyInt(&context->java, VRAPI_SYS_PROP_SUGGESTED_EYE_TEXTURE_HEIGHT);

    // ===== Create Swapchain =====

    this->swapchain = vrapi_CreateTextureSwapChain3(VRAPI_TEXTURE_TYPE_2D_ARRAY, VK_FORMAT_R8G8B8A8_UNORM,
                                                    this->extent.width, this->extent.height, 1, 3);

    this->length = vrapi_GetTextureSwapChainLength(this->swapchain);

    this->images.resize(this->length);
    this->imageviews.resize(this->length);

    for (int i = 0; i < this->length; i++)
    {
        this->images[i] = vrapi_GetTextureSwapChainBufferVulkan(this->swapchain, i);
        createVkImageView(this->context->physicalDevice, this->context->device, this->images[i], VK_IMAGE_VIEW_TYPE_2D_ARRAY,
                          this->colorFormat, 1, 2, VK_IMAGE_ASPECT_COLOR_BIT, &this->imageviews[i]);
    }

    // ===== MSAA Color Resolve Image =====

    createVkImage(context, VK_IMAGE_TYPE_2D, colorFormat,
                    extent, 1, 2, sample_count, VK_IMAGE_TILING_OPTIMAL,
                    VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
                    VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &colorImage, &colorImageMemory);

    transitionImageLayout(context->device, context->primaryGraphicsQueue->queue, context->primaryGraphicsQueue->commandPool, colorImage, colorFormat, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, 1);

    createVkImageView(context->physicalDevice, context->device, colorImage, VK_IMAGE_VIEW_TYPE_2D_ARRAY,
                        colorFormat, 1, 2, VK_IMAGE_ASPECT_COLOR_BIT,
                        &colorImageView);

    // ===== Depth Image =====

    createVkImage(context, VK_IMAGE_TYPE_2D, depthFormat,
                    extent, 1, 2, sample_count, VK_IMAGE_TILING_OPTIMAL,
                    VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
                    VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &depthImage, &depthImageMemory);

    transitionImageLayout(context->device, context->primaryGraphicsQueue->queue, context->primaryGraphicsQueue->commandPool, depthImage, depthFormat, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, 1);

    createVkImageView(context->physicalDevice, context->device, depthImage, VK_IMAGE_VIEW_TYPE_2D_ARRAY,
                        depthFormat, 1, 2, VK_IMAGE_ASPECT_DEPTH_BIT, &depthImageView);

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

OVRRenderer::~OVRRenderer()
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

    vrapi_DestroyTextureSwapChain(swapchain);

    DEBUG("RENDERER - Renderer Destroyed");
}