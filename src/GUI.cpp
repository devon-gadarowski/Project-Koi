#include <RenderFramework.h>

using namespace RenderFramework;

void GUI::fpsMeter(long elapsedTime)
{
	timeBeforeFPSUpdate -= elapsedTime;
	if (timeBeforeFPSUpdate < 0)
	{
		FPS = frameCount;
		frameCount = 0;
		timeBeforeFPSUpdate = 1000;
	}

    ImVec2 window_pos = ImVec2(10.0f, 10.0f);
    ImVec2 window_pos_pivot = ImVec2(0.0f, 0.0f);
    ImGui::SetNextWindowPos(window_pos, ImGuiCond_Always, window_pos_pivot);

	ImGui::SetNextWindowBgAlpha(0.35f);
	ImGui::Begin("FPS", nullptr, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoDecoration |ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav);
    ImGui::Text("FPS: %u", FPS);

    ImGui::End();
}

void GUI::console()
{
	ImGui::Begin("Console");
	const float footer_height_to_reserve = ImGui::GetStyle().ItemSpacing.y + ImGui::GetFrameHeightWithSpacing();
	ImGui::BeginChild("ScrollingRegion", ImVec2(0, -footer_height_to_reserve), false, ImGuiWindowFlags_HorizontalScrollbar);

	for (int i = history.size()-1; i >= 0; i--)
		ImGui::Text("%s", history[i].c_str());

	ImGui::EndChild();
	ImGui::Separator();
	const ImGuiInputTextFlags flags = ImGuiInputTextFlags_EnterReturnsTrue|ImGuiInputTextFlags_CallbackCompletion|ImGuiInputTextFlags_CallbackHistory;
    if (ImGui::InputText("Test", buffer, 30, flags, [](ImGuiInputTextCallbackData * data)->int { return ((GUI *) data->UserData)->onConsoleUpdate(data); }, (void *) this))
	{
		history.push_back(std::string(buffer));
		buffer[0] = '\0';

		msgBus->sendMessage(ProcessCommand, history[historyIndex]);

		historyIndex++;
	}

	ImGui::End();
}

int GUI::onConsoleUpdate(ImGuiInputTextCallbackData * data)
{
	if (data->EventKey == ImGuiKey_UpArrow)
	{
		if (historyIndex > 0)
			historyIndex--;
	}
	else if (data->EventKey == ImGuiKey_DownArrow)
	{
		if (historyIndex < history.size())
			historyIndex++;
	}
	else if (data->EventKey == ImGuiKey_Tab)
	{
		// TODO: Implement Completion
	}

	if (historyIndex == history.size())
	{
		strcpy(data->Buf, "");
		data->BufTextLen = 0;
	}
	else
	{
		strcpy(data->Buf, history[historyIndex].c_str());
		data->BufTextLen = history[historyIndex].length();
	}

	data->BufDirty = true;

	return 0;
}

void GUI::update(long elapsedTime)
{
	ImGui_ImplVulkan_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();

	//ImGui::ShowDemoWindow();
	fpsMeter(elapsedTime);
	console();

	ImGui::Render();

	frameCount++;
}

VkCommandBuffer GUI::getFrame(uint32_t imageIndex)
{
	vkResetCommandPool(context->device, commandPools[imageIndex], 0);
	VkCommandBufferBeginInfo info = {};
	info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	info.flags |= VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
	vkBeginCommandBuffer(commandBuffers[imageIndex], &info);

	VkRenderPassBeginInfo passInfo = {};
	passInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	passInfo.renderPass = renderPass;
	passInfo.framebuffer = framebuffers[imageIndex];
	passInfo.renderArea.extent.width = renderer->extent.width;
	passInfo.renderArea.extent.height = renderer->extent.height;
	passInfo.clearValueCount = 0;
	passInfo.pClearValues = nullptr;
	vkCmdBeginRenderPass(commandBuffers[imageIndex], &passInfo, VK_SUBPASS_CONTENTS_INLINE);

	ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), commandBuffers[imageIndex]);

	vkCmdEndRenderPass(commandBuffers[imageIndex]);
	vkEndCommandBuffer(commandBuffers[imageIndex]);

	return commandBuffers[imageIndex];
}

GUI::GUI(Context * context, Renderer * renderer, MessageBus * msgBus)
{
	this->context = context;
	this->renderer = renderer;
	this->msgBus = msgBus;

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

	frameCount = 0;
	FPS = 0;
	timeBeforeFPSUpdate = 1000;

	buffer[0] = '\0';
	historyIndex = 0;

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
