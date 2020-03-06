
#include <RenderFramework.h>

using namespace RenderFramework;

uint32_t Renderer::getNextImage()
{
	uint32_t imageIndex;
    vkAcquireNextImageKHR(context->device, swapchain, UINT64_MAX, imageAvailable, VK_NULL_HANDLE, &imageIndex);

	vkWaitForFences(context->device, 1, &fences[imageIndex], VK_TRUE, std::numeric_limits<uint64_t>::max());
	vkResetFences(context->device, 1, &fences[imageIndex]);

	return imageIndex;
}

void Renderer::render(uint32_t imageIndex, std::vector<VkCommandBuffer> commandBuffers)
{
	VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};

	VkSubmitInfo submitInfo = {};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.waitSemaphoreCount = 1;
	submitInfo.pWaitSemaphores = &imageAvailable;
	submitInfo.pWaitDstStageMask = waitStages;
	submitInfo.commandBufferCount = commandBuffers.size();
	submitInfo.pCommandBuffers = commandBuffers.data();
	submitInfo.signalSemaphoreCount = 1;
	submitInfo.pSignalSemaphores = &renderFinished;

	VkResult result = vkQueueSubmit(context->graphicsQueue.queue, 1, &submitInfo, fences[imageIndex]);
	if (result != VK_SUCCESS)
	{
		PANIC("RENDERER - Failed to submit draw command buffer %d", result);
	}
}

void Renderer::present(uint32_t imageIndex)
{
	VkResult result = VK_SUCCESS;

	VkPresentInfoKHR presentInfo = {};
	presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	presentInfo.pNext = nullptr;
	presentInfo.waitSemaphoreCount = 1;
	presentInfo.pWaitSemaphores = &renderFinished;
	presentInfo.swapchainCount = 1;
	presentInfo.pSwapchains = &swapchain;
	presentInfo.pImageIndices = &imageIndex;
	presentInfo.pResults = &result;

	if (result != VK_SUCCESS)
	{
		PANIC("RENDERER - Failed to present swapchain image %d", result);
	}

	vkQueuePresentKHR(context->presentQueue.queue, &presentInfo);
}

Renderer::Renderer(Context * context)
{
	this->context = context;

	getSwapchainFormats(context->physicalDevice, context->surface, &colorFormat, &colorSpace, &depthFormat);
	getSwapchainPresentMode(context->physicalDevice, context->surface, &presentMode);
	getSwapchainExtent(context->physicalDevice, context->surface, &extent);
	getSwapchainLength(context->physicalDevice, context->surface, &length);

	createVkRenderPass(context->device, colorFormat, depthFormat, SAMPLE_COUNT, &renderPass);

	createSwapchain();

	DEBUG("RENDER_FRAMEWORK - Renderer Created");
}

Renderer::~Renderer()
{
	destroySwapchain();
	destroyVkRenderPass(context->device, &renderPass);

	DEBUG("RENDER_FRAMEWORK - Renderer Destroyed");
}

void Renderer::createSwapchain()
{
	uint32_t queueIndices [] = {context->graphicsQueue.index, context->transferQueue.index};

	createVkSwapchainKHR(context->physicalDevice, context->device, context->surface, length, colorFormat,
	                     colorSpace, extent, queueIndices, context->queueCount, presentMode, &swapchain);

	vkGetSwapchainImagesKHR(context->device, swapchain, &length, nullptr);
	images.resize(length);
	vkGetSwapchainImagesKHR(context->device, swapchain, &length, images.data());

	imageViews.resize(length);
	for (int i = 0; i < length; i++)
	{
		createVkImageView(context->physicalDevice, context->device, images[i], VK_IMAGE_VIEW_TYPE_2D,
			              colorFormat, MIP_LEVELS, 1, VK_IMAGE_ASPECT_COLOR_BIT, &imageViews[i]);
	}

	if (SAMPLE_COUNT > 1)
	{
		createVkImage(context, VK_IMAGE_TYPE_2D, colorFormat,
			          extent, MIP_LEVELS, 1, SAMPLE_COUNT, VK_IMAGE_TILING_OPTIMAL,
			          VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
			          VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &colorImage, &colorImageMemory);

		transitionImageLayout(context, colorImage, colorFormat, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, 1);

		createVkImageView(context->physicalDevice, context->device, colorImage, VK_IMAGE_VIEW_TYPE_2D,
			              colorFormat, MIP_LEVELS, 1, VK_IMAGE_ASPECT_COLOR_BIT,
			              &colorImageView);
	}
	if (depthFormat != VK_FORMAT_UNDEFINED)
	{
		createVkImage(context, VK_IMAGE_TYPE_2D, depthFormat,
				      extent, MIP_LEVELS, 1, SAMPLE_COUNT, VK_IMAGE_TILING_OPTIMAL,
				      VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
				      VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &depthImage, &depthImageMemory);

		transitionImageLayout(context, depthImage, depthFormat, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, 1);

		createVkImageView(context->physicalDevice, context->device, depthImage, VK_IMAGE_VIEW_TYPE_2D,
				          depthFormat, MIP_LEVELS, 1, VK_IMAGE_ASPECT_DEPTH_BIT, &depthImageView);
	}

	framebuffers.resize(length);
	for (int i = 0; i < length; i++)
	{
		createVkFramebuffer(context->device, nullptr, 0, renderPass, colorImageView, depthImageView,
		                    imageViews[i], extent.width, extent.height, 1, &framebuffers[i]);
	}

	fences.resize(length);
	for (int i = 0; i < length; i++)
	{
		createVkFence(context->device, nullptr, VK_FENCE_CREATE_SIGNALED_BIT, &fences[i]);
	}

	VkSemaphoreCreateInfo semaphoreInfo = {};
	semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

	vkCreateSemaphore(context->device, &semaphoreInfo, nullptr, &imageAvailable);
	vkCreateSemaphore(context->device, &semaphoreInfo, nullptr, &renderFinished);
}

void Renderer::destroySwapchain()
{
	vkDestroySemaphore(context->device, imageAvailable, nullptr);
	vkDestroySemaphore(context->device, renderFinished, nullptr);

	for (int i = 0; i < length; i++)
	{
		vkDestroyFence(context->device, fences[i], nullptr);
		vkDestroyImageView(context->device, imageViews[i], nullptr);
		vkDestroyFramebuffer(context->device, framebuffers[i], nullptr);
	}

	if (colorImage != VK_NULL_HANDLE)
	{
		vkDestroyImageView(context->device, colorImageView, nullptr);
		vkDestroyImage(context->device, colorImage, nullptr);
		vkFreeMemory(context->device, colorImageMemory, nullptr);
	}

	if (depthImage != VK_NULL_HANDLE)
	{
		vkDestroyImageView(context->device, depthImageView, nullptr);
		vkDestroyImage(context->device, depthImage, nullptr);
		vkFreeMemory(context->device, depthImageMemory, nullptr);
	}

	destroyVkSwapchainKHR(context->device, &swapchain);
}
