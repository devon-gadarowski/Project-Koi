
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_vulkan.h>

#include <RenderFramework.h>

using namespace RenderFramework;

void GUI::update(long elapsedTime)
{
	ImGui_ImplVulkan_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();

	//ImGui::ShowDemoWindow();
    ImGui::Begin("FPS");
    ImGui::Text("%0.1f", 6000.0 / elapsedTime);
    ImGui::End();

	ImGui::Render();
}

VkCommandBuffer GUI::getFrame(uint32_t imageIndex)
{
	vkResetCommandPool(context->device, commandPools[imageIndex], 0);
	VkCommandBufferBeginInfo info = {};
	info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	info.flags |= VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
	vkBeginCommandBuffer(commandBuffers[imageIndex], &info);

	vkCmdPipelineBarrier(
		commandBuffers[imageIndex],
		VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
		VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
		0,
		0, nullptr,
		0, nullptr,
		0, nullptr);

	VkClearValue clearColors[3];
	clearColors[0].color = {0.0f, 0.0f, 0.0f, 1.0f};
	clearColors[1].depthStencil = {1.0f, 0};
	clearColors[2].color = {0.0f, 0.0f, 0.0f, 1.0f};

	VkRenderPassBeginInfo passInfo = {};
	passInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	passInfo.renderPass = renderPass;
	passInfo.framebuffer = framebuffers[imageIndex];
	passInfo.renderArea.extent.width = renderer->extent.width;
	passInfo.renderArea.extent.height = renderer->extent.height;
	passInfo.clearValueCount = 1;
	passInfo.pClearValues = clearColors;
	vkCmdBeginRenderPass(commandBuffers[imageIndex], &passInfo, VK_SUBPASS_CONTENTS_INLINE);

	ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), commandBuffers[imageIndex]);

	vkCmdEndRenderPass(commandBuffers[imageIndex]);
	vkEndCommandBuffer(commandBuffers[imageIndex]);

	return commandBuffers[imageIndex];
}

void GUI::draw()
{
	uint32_t imageIndex = renderer->frameIndex % renderer->length;

	//vkQueueWaitIdle(context->graphicsQueue.queue);

	vkWaitForFences(context->device, 1, &renderer->fences[imageIndex], VK_TRUE, std::numeric_limits<uint64_t>::max());
	vkResetFences(context->device, 1, &renderer->fences[imageIndex]);

	vkResetCommandPool(context->device, commandPools[imageIndex], 0);
	VkCommandBufferBeginInfo info = {};
	info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	info.flags |= VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
	vkBeginCommandBuffer(commandBuffers[imageIndex], &info);

	vkCmdPipelineBarrier(
		commandBuffers[imageIndex],
		VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
		VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
		0,
		0, nullptr,
		0, nullptr,
		0, nullptr);

	VkClearValue clearColors[3];
	clearColors[0].color = {0.0f, 0.0f, 0.0f, 1.0f};
	clearColors[1].depthStencil = {1.0f, 0};
	clearColors[2].color = {0.0f, 0.0f, 0.0f, 1.0f};

	VkRenderPassBeginInfo passInfo = {};
	passInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	passInfo.renderPass = renderPass;
	passInfo.framebuffer = framebuffers[imageIndex];
	passInfo.renderArea.extent.width = renderer->extent.width;
	passInfo.renderArea.extent.height = renderer->extent.height;
	passInfo.clearValueCount = 1;
	passInfo.pClearValues = clearColors;
	vkCmdBeginRenderPass(commandBuffers[imageIndex], &passInfo, VK_SUBPASS_CONTENTS_INLINE);

	ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), commandBuffers[imageIndex]);

	// Submit command buffer
	vkCmdEndRenderPass(commandBuffers[imageIndex]);
	vkEndCommandBuffer(commandBuffers[imageIndex]);

	VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};

	VkSemaphore waitSemaphores[] = {renderer->imageAvailable, renderer->renderFinished};

	VkSubmitInfo submitInfo = {};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.waitSemaphoreCount = 1;
	submitInfo.pWaitSemaphores = &renderer->renderFinished;
	submitInfo.pWaitDstStageMask = waitStages;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &commandBuffers[imageIndex];
	submitInfo.signalSemaphoreCount = 1;
	submitInfo.pSignalSemaphores = &renderer->guiFinished;

	VkResult result = vkQueueSubmit(context->graphicsQueue.queue, 1, &submitInfo, renderer->fences[imageIndex]);
	if (result != VK_SUCCESS)
	{
		PANIC("RENDERER - Failed to submit draw command buffer %d", result);
	}
}

GUI::GUI(Context * context, Renderer * renderer)
{
	this->context = context;
	this->renderer = renderer;

	// Create imgui context
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;

	ImGui::StyleColorsDark();

	// Create Descriptor Pool specific to imgui
	VkDescriptorPoolSize pool_sizes[] =
	{
		{ VK_DESCRIPTOR_TYPE_SAMPLER, 1000 },
		{ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000 },
		{ VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1000 },
		{ VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1000 },
		{ VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1000 },
		{ VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1000 },
		{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000 },
		{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1000 },
		{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000 },
		{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1000 },
		{ VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1000 }
	};

	VkDescriptorPoolCreateInfo pool_info = {};
	pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	pool_info.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
	pool_info.maxSets = 1000 * IM_ARRAYSIZE(pool_sizes);
	pool_info.poolSizeCount = (uint32_t)IM_ARRAYSIZE(pool_sizes);
	pool_info.pPoolSizes = pool_sizes;
	vkCreateDescriptorPool(context->device, &pool_info, nullptr, &descriptorPool);

	// Create render pass for imgui
	createVkRenderPassOverlay(context->device, renderer->colorFormat, &renderPass);

	// Initialize imgui with renderer objects
	ImGui_ImplGlfw_InitForVulkan(context->window, true);
	ImGui_ImplVulkan_InitInfo init_info = {};
	init_info.Instance = context->instance;
	init_info.PhysicalDevice = context->physicalDevice;
	init_info.Device = context->device;
	init_info.QueueFamily = context->graphicsQueue.index;
	init_info.Queue = context->graphicsQueue.queue;
	init_info.PipelineCache = VK_NULL_HANDLE;
	init_info.DescriptorPool = descriptorPool;
	init_info.Allocator = nullptr;
	init_info.MinImageCount = renderer->length;
	init_info.ImageCount = renderer->length;
	init_info.CheckVkResultFn = nullptr;
	ImGui_ImplVulkan_Init(&init_info, renderPass);

	commandPools.resize(renderer->length);
	commandBuffers.resize(renderer->length);
	framebuffers.resize(renderer->length);
	for (int i = 0; i < renderer->length; i++)
	{
		createVkCommandPool(context->device, nullptr, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT, context->graphicsQueue.index, &commandPools[i]);
		allocateVkCommandBuffers(context->device, nullptr, commandPools[i], VK_COMMAND_BUFFER_LEVEL_PRIMARY, 1, &commandBuffers[i]);
		createVkFramebuffer(context->device, nullptr, 0, renderPass, nullptr, nullptr, renderer->imageViews[i], renderer->extent.width, renderer->extent.height, 1, &framebuffers[i]);
	}

	// Upload Fonts to GPU
	VkCommandBuffer command_buffer = beginSingleTimeCommands(context);
	ImGui_ImplVulkan_CreateFontsTexture(command_buffer);
	endSingleTimeCommands(context, command_buffer);

	DEBUG("GUI - GUI Created");
}

GUI::~GUI()
{
	vkQueueWaitIdle(context->graphicsQueue.queue);
	vkQueueWaitIdle(context->presentQueue.queue);

	for (int i = 0; i < renderer->length; i++)
		vkDestroyFramebuffer(context->device, framebuffers[i], nullptr);

	for (int i = 0; i < renderer->length; i++)
		vkDestroyCommandPool(context->device, commandPools[i], nullptr);

	destroyVkRenderPass(context->device, &renderPass);
	vkDestroyDescriptorPool(context->device, descriptorPool, nullptr);

	ImGui_ImplVulkan_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

	DEBUG("GUI - GUI Destroyed");
}
