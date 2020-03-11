#include <render/GUI.h>
#include <render/Utilities.h>

GUIElement::GUIElement()
{

}

GUIElement::~GUIElement()
{

}

FPSMeter::FPSMeter()
{

}

FPSMeter::~FPSMeter()
{

}

void FPSMeter::init()
{
    this->FPS = 0;
    this->frameCount = 0;
    this->timeBeforeFPSUpdate = 1000;
}

void FPSMeter::update(long elapsedTime)
{
    this->frameCount++;
    this->timeBeforeFPSUpdate -= elapsedTime;
	if (timeBeforeFPSUpdate < 0)
	{
		this->FPS = this->frameCount;
		this->frameCount = 0;
		this->timeBeforeFPSUpdate = 1000;
	}
}

void FPSMeter::draw()
{
    ImVec2 window_pos = ImVec2(10.0f, 10.0f);
    ImVec2 window_pos_pivot = ImVec2(0.0f, 0.0f);
    ImGui::SetNextWindowPos(window_pos, ImGuiCond_Always, window_pos_pivot);

	ImGui::SetNextWindowBgAlpha(0.35f);
	ImGui::Begin("FPS", nullptr, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoDecoration |ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav);
	ImGui::SetWindowFontScale(1.5f);
    ImGui::Text("FPS: %u", FPS);
    ImGui::End();
}

Console::Console()
{

}

Console::~Console()
{

}

void Console::init()
{
	buffer[0] = '\0';
	historyIndex = 0;

	commands[hashCode("exit")] = &Console::exit;
    commands[hashCode("add")] = &Console::addModel;
}

void Console::update(long elapsedTime)
{

}

void Console::draw()
{
	ImGui::Begin("Console");
	ImGui::SetWindowFontScale(1.5f);
	const float footer_height_to_reserve = ImGui::GetStyle().ItemSpacing.y + ImGui::GetFrameHeightWithSpacing();
	ImGui::BeginChild("ScrollingRegion", ImVec2(0, -footer_height_to_reserve), false, ImGuiWindowFlags_HorizontalScrollbar);

	for (int i = history.size()-1; i >= 0; i--)
		ImGui::Text("%s", history[i].c_str());

	ImGui::EndChild();
	ImGui::Separator();
	ImGui::SetItemDefaultFocus();
	const ImGuiInputTextFlags flags = ImGuiInputTextFlags_EnterReturnsTrue|ImGuiInputTextFlags_CallbackCompletion|ImGuiInputTextFlags_CallbackHistory;
    if (ImGui::InputText("", buffer, 128, flags, [](ImGuiInputTextCallbackData * data)->int { return ((Console *) data->UserData)->onConsoleUpdate(data); }, (void *) this))
	{
		history.push_back(std::string(buffer));
		buffer[0] = '\0';

		this->processCommand(history[historyIndex]);

		historyIndex++;
	}

	ImGui::End();
}

int Console::onConsoleUpdate(ImGuiInputTextCallbackData * data)
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

void Console::processCommand(std::string command)
{
	if (command.length() == 0)
		return;

	std::vector<std::string> args = parseCommandArgs(command);

	long hash = hashCode(args[0]);
	auto action = commands.find(hash);

	if (action != commands.end())
		(this->*(action->second))(args);
    else
        DEBUG("CONSOLE - Unknown Command %s", args[0].c_str());    
    
}

std::vector<std::string> Console::parseCommandArgs(std::string command)
{
	std::vector<std::string> args = {};

	int i = 0;
	int size = 0;
	while (i < command.length())
	{
		args.push_back(std::string());
		int j = 0;
		while (i < command.length())
		{
			if (command[i] == ' ' || command[i] == '\n')
			{
				i++;
				break;
			}

			args[size].push_back(command[i++]);
		}
		size++;
	}

	return args;
}

long Console::hashCode(std::string command)
{
	long hash = 0;
	long powah = 31;
	for (int i = 0; i < command.length(); i++)
		hash = hash * powah + command[i];

	return hash;
}	

void Console::exit(std::vector<std::string> args)
{
	app->sendMessage(Exit);
}

void Console::addModel(std::vector<std::string> args)
{
    if (args.size() >= 3)
		app->sendMessageNow(AddModel, &args);

	if (args.size() == 2)
	{
		args.push_back(findFile(args[1], "assets/meshes/"));
		app->sendMessageNow(AddModel, &args);
	}
}

LightingTweaker::LightingTweaker()
{

}

LightingTweaker::~LightingTweaker()
{

}

void LightingTweaker::init()
{

}

void LightingTweaker::update(long elapsedTime)
{
    app->sendMessage(SetLighting, &this->data);
}

void LightingTweaker::draw()
{
	ImGui::Begin("Lighting Tweaker");
	ImGui::SetWindowFontScale(1.5f);

    ImGui::InputFloat3("Direction", (float*)&data.direction);
    ImGui::ColorEdit4("Ambient", (float*)&data.ambient);
    ImGui::ColorEdit4("Diffuse", (float*)&data.diffuse);
    ImGui::ColorEdit4("Specular", (float*)&data.specular);

	ImGui::End();
}

ModelViewer::ModelViewer()
{

}

ModelViewer::~ModelViewer()
{

}

void ModelViewer::init()
{
    getModels(nullptr);

    this->setMessageCallback(AddModel, (message_method_t) &ModelViewer::getModels);
}

void ModelViewer::update(long elapsedTime)
{

}

void ModelViewer::draw()
{
	ImGui::Begin("Model Viewer");
	ImGui::SetWindowFontScale(1.5f);

    for (auto & model : this->models)
    {
        ImGui::Text("%u %s (%u)\n", model._id, model.name.c_str(), model.instanceCount);
    }

	ImGui::End();
}

void ModelViewer::getModels(Message * msg)
{
    this->models.clear();
    app->sendMessageNow(GetModelData, &this->models);
}

GUI::GUI(DesktopContext * context, DesktopRenderer * renderer, MessageBus * app)
{
	this->context = context;
	this->renderer = renderer;

	// ===== Create imgui context =====

	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;

	ImGui::StyleColorsDark();

	// ===== Create Descriptor Pool specific to imgui =====

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

	// ===== Create render pass for imgui =====

	VkAttachmentDescription colorAttachment = {};
	colorAttachment.flags = 0;
	colorAttachment.format = renderer->colorFormat;
	colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
	colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
	colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	colorAttachment.initialLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
	colorAttachment.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	VkAttachmentReference colorAttachmentRef = {};
	colorAttachmentRef.attachment = 0;
	colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	VkSubpassDescription subpass = {};
	subpass.flags = 0;
	subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpass.inputAttachmentCount = 0;
	subpass.pInputAttachments = nullptr;
	subpass.colorAttachmentCount = 1;
	subpass.pColorAttachments = &colorAttachmentRef;
	subpass.pResolveAttachments = nullptr;
	subpass.pDepthStencilAttachment = nullptr;
	subpass.preserveAttachmentCount = 0;
	subpass.pPreserveAttachments = nullptr;

	VkRenderPassCreateInfo renderPassInfo = {};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	renderPassInfo.pNext = nullptr;
	renderPassInfo.flags = 0;
	renderPassInfo.attachmentCount = 1;
	renderPassInfo.pAttachments = &colorAttachment;
	renderPassInfo.subpassCount = 1;
	renderPassInfo.pSubpasses = &subpass;
	renderPassInfo.dependencyCount = 0;
	renderPassInfo.pDependencies = nullptr;

	int result = vkCreateRenderPass(context->device, &renderPassInfo, nullptr, &this->renderPass);
    VALIDATE(result == VK_SUCCESS, "Failed to create ImGUI VkRenderPass");

	// Initialize imgui with renderer objects
	ImGui_ImplGlfw_InitForVulkan(context->window, true);
	ImGui_ImplVulkan_InitInfo init_info = {};
	init_info.Instance = context->instance;
	init_info.PhysicalDevice = context->physicalDevice;
	init_info.Device = context->device;
	init_info.QueueFamily = context->primaryGraphicsQueue->index;
	init_info.Queue = context->primaryGraphicsQueue->queue;
	init_info.PipelineCache = VK_NULL_HANDLE;
	init_info.DescriptorPool = descriptorPool;
	init_info.Allocator = nullptr;
	init_info.MinImageCount = renderer->length;
	init_info.ImageCount = renderer->length;
	init_info.CheckVkResultFn = nullptr;
	ImGui_ImplVulkan_Init(&init_info, renderPass);

    this->framebuffers.resize(renderer->length);
	for (int i = 0; i < this->framebuffers.size(); i++)
	{
        createVkFramebuffer(context->device, nullptr, 0, renderPass, nullptr, nullptr, renderer->imageviews[i], renderer->extent.width, renderer->extent.height, 1, &framebuffers[i]);
	}

	// Upload Fonts to GPU
	VkCommandBuffer command_buffer = beginSingleTimeCommands(context->device, context->primaryGraphicsQueue->commandPool);
	ImGui_ImplVulkan_CreateFontsTexture(command_buffer);
	endSingleTimeCommands(context->device, context->primaryGraphicsQueue->queue, context->primaryGraphicsQueue->commandPool, command_buffer);

    FPSMeter * fpsmeter = new FPSMeter();
    Console * console = new Console();
    LightingTweaker * lightingTweaker = new LightingTweaker();
    ModelViewer * modelViewer = new ModelViewer();

    app->registerSystem(fpsmeter);
    app->registerSystem(console);
    app->registerSystem(lightingTweaker);
    app->registerSystem(modelViewer);

    elements.push_back(fpsmeter);
    elements.push_back(console);
    elements.push_back(lightingTweaker);
    elements.push_back(modelViewer);

	DEBUG("GUI - GUI Created");
}

GUI::~GUI()
{
    for (auto & element : elements)
        delete element;

    elements.clear();

	for (int i = 0; i < renderer->length; i++)
		vkDestroyFramebuffer(context->device, framebuffers[i], nullptr);

	vkDestroyRenderPass(context->device, renderPass, nullptr);
	vkDestroyDescriptorPool(context->device, descriptorPool, nullptr);

	ImGui_ImplVulkan_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

	DEBUG("GUI - GUI Destroyed");
}

void GUI::draw(VkCommandBuffer commandbuffer)
{
	ImGui_ImplVulkan_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();

    for (auto & element : elements)
        element->draw();

    ImGui::Render();

	VkRenderPassBeginInfo passInfo = {};
	passInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	passInfo.renderPass = renderPass;
	passInfo.framebuffer = framebuffers[renderer->currentImageIndex];
	passInfo.renderArea.extent.width = renderer->extent.width;
	passInfo.renderArea.extent.height = renderer->extent.height;
	passInfo.clearValueCount = 0;
	passInfo.pClearValues = nullptr;
	vkCmdBeginRenderPass(commandbuffer, &passInfo, VK_SUBPASS_CONTENTS_INLINE);

	ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), commandbuffer);

	vkCmdEndRenderPass(commandbuffer);
}