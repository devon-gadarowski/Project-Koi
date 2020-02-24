
#include <RenderFramework.h>

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <chrono>

using namespace RenderFramework;

void Scene3D::updateUBO()
{
	glm::mat4 view = glm::lookAt(
		glm::vec3(camera.position.x, camera.position.y, camera.position.z),
		glm::vec3(camera.position.x + camera.direction.x, camera.position.y + camera.direction.y, camera.position.z + camera.direction.z),
		glm::vec3(camera.up.x, camera.up.y, camera.up.z)
	);

	glm::mat4 proj = glm::perspective(glm::radians(45.0f), renderer->extent.width / (float) renderer->extent.height, 0.1f, 10.0f);

	proj[1][1] *= -1;

	for (int i = 0; i < 4; i++)
	{
		for (int j = 0; j < 4; j++)
		{
			ubo.view[i][j] = view[i][j];
			ubo.proj[i][j] = proj[i][j];
		}
	}

	void* data;
	vkMapMemory(context->device, uniformBuffersMemory[renderer->frameIndex % renderer->length], 0, sizeof(ubo), 0, &data);
	memcpy(data, &ubo, sizeof(ubo));
	vkUnmapMemory(context->device, uniformBuffersMemory[renderer->frameIndex % renderer->length]);
}

void Scene3D::draw()
{
	uint32_t imageIndex = renderer->frameIndex % renderer->length;

    vkAcquireNextImageKHR(context->device, renderer->swapchain, UINT64_MAX, renderer->imageAvailable, VK_NULL_HANDLE, &imageIndex);

	vkWaitForFences(context->device, 1, &renderer->fences[imageIndex], VK_TRUE, std::numeric_limits<uint64_t>::max());
	vkResetFences(context->device, 1, &renderer->fences[imageIndex]);

	VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};

	VkSubmitInfo submitInfo = {};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.waitSemaphoreCount = 1;
	submitInfo.pWaitSemaphores = &renderer->imageAvailable;
	submitInfo.pWaitDstStageMask = waitStages;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &commandBuffers[imageIndex];
	submitInfo.signalSemaphoreCount = 1;
	submitInfo.pSignalSemaphores = &renderer->renderFinished;

	VkResult result = vkQueueSubmit(context->graphicsQueue.queue, 1, &submitInfo, renderer->fences[imageIndex]);
	if (result != VK_SUCCESS)
	{
		PANIC("RENDERER - Failed to submit draw command buffer %d", result);
	}

	VkPresentInfoKHR presentInfo = {};
	presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	presentInfo.pNext = nullptr;
	presentInfo.waitSemaphoreCount = 1;
	presentInfo.pWaitSemaphores = &renderer->renderFinished;
	presentInfo.swapchainCount = 1;
	presentInfo.pSwapchains = &renderer->swapchain;
	presentInfo.pImageIndices = &imageIndex;
	presentInfo.pResults = &result;

	if (result != VK_SUCCESS)
	{
		PANIC("RENDERER - Failed to present swapchain image %d", result);
	}

	vkQueuePresentKHR(context->presentQueue.queue, &presentInfo);

	renderer->frameIndex++;
}

Scene3D::Scene3D(Context * context, Renderer * renderer, std::string filename)
{
	this->context = context;
	this->renderer = renderer;

	std::vector<std::string> diffuseTextureNames;

	RenderFramework::loadSceneModels(filename, &meshes, &diffuseTextureNames);

	for (auto& mesh : meshes)
	{
		INFO("SCENE3D - Loaded model %s with %lu vertices", mesh.name.c_str(), mesh.vertices.size());

		// Create Vertex/Index/Instance Buffers
		RenderFramework::createMeshBuffers(context, &mesh);
	}

	// Load Shaders
	RenderFramework::createShaderModule(context, "../assets/shaders/vert.spv", &vertexShader);
	RenderFramework::createShaderModule(context, "../assets/shaders/frag.spv", &fragmentShader);

	// Load Textures
	textureImages.resize(diffuseTextureNames.size());
	textureImageMemorys.resize(diffuseTextureNames.size());
	textureImageViews.resize(diffuseTextureNames.size());
	int i = 0;
	for (auto& dTextureName : diffuseTextureNames)
	{
		loadMeshTexture(context, renderer, dTextureName.c_str(), &textureImages[i], &textureImageMemorys[i], &textureImageViews[i]);
		i++;
	}

	createMeshTextureSampler(context->device, &textureSampler);

	// Create Uniform Buffers
	uniformBuffers.resize(renderer->length);
	uniformBuffersMemory.resize(renderer->length);
	for (int i = 0; i < uniformBuffers.size(); i++)
	{
		createBuffer(context, sizeof(UBO), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
		             VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &uniformBuffers[i], &uniformBuffersMemory[i]);
	}

	// Create Descriptors
	setupDescriptors();

	// Create Pipeline
	createPipeline();
	recordCommands();

	updateUBO();

	DEBUG("SCENE3D - Scene Created %s", filename.c_str());
}

Scene3D::~Scene3D()
{
	vkQueueWaitIdle(context->graphicsQueue.queue);

	vkFreeCommandBuffers(context->device, context->graphicsCommandPool, renderer->length, commandBuffers.data());

	vkDestroyPipeline(context->device, graphicsPipeline, nullptr);
	vkDestroyPipelineLayout(context->device, pipelineLayout, nullptr);

	for (int i = 0; i < uniformBuffers.size(); i++)
	{
		vkDestroyBuffer(context->device, uniformBuffers[i], nullptr);
		vkFreeMemory(context->device, uniformBuffersMemory[i], nullptr);
	}

	vkDestroyDescriptorSetLayout(context->device, descriptorSetLayout, nullptr);
	vkDestroyDescriptorPool(context->device, descriptorPool, nullptr);

	vkDestroySampler(context->device, textureSampler, nullptr);

	for (int i = 0; i < textureImages.size(); i++)
	{
		vkDestroyImage(context->device, textureImages[i], nullptr);
		vkFreeMemory(context->device, textureImageMemorys[i], nullptr);
		vkDestroyImageView(context->device, textureImageViews[i], nullptr);
	}

	for (auto& mesh : meshes)
	{
		vkDestroyBuffer(context->device, mesh.vertexBuffer, nullptr);
		vkFreeMemory(context->device, mesh.vertexMemory, nullptr);

		vkDestroyBuffer(context->device, mesh.indexBuffer, nullptr);
		vkFreeMemory(context->device, mesh.indexMemory, nullptr);

		vkDestroyBuffer(context->device, mesh.instanceBuffer, nullptr);
		vkFreeMemory(context->device, mesh.instanceMemory, nullptr);
	}

	vkDestroyShaderModule(context->device, vertexShader, nullptr);
	vkDestroyShaderModule(context->device, fragmentShader, nullptr);


	DEBUG("SCENE3D - Scene Destroyed");
}

void Scene3D::getVertexInputDescriptions(std::vector<VkVertexInputBindingDescription> * bindingDesc,
                                         std::vector<VkVertexInputAttributeDescription> * attribDesc)
{
	bindingDesc->emplace_back();
	bindingDesc->back().binding = 0;
	bindingDesc->back().stride = sizeof(Vertex);
	bindingDesc->back().inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
	Vertex::getAttributeDescriptions(0, attribDesc);

	bindingDesc->emplace_back();
	bindingDesc->back().binding = 1;
	bindingDesc->back().stride = sizeof(Instance);
	bindingDesc->back().inputRate = VK_VERTEX_INPUT_RATE_INSTANCE;
	Instance::getAttributeDescriptions(1, attribDesc);
}

void Scene3D::setupDescriptors()
{
	std::vector<VkDescriptorSetLayoutBinding> bindings(2);

	bindings[0] = getDescriptorSetLayoutBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_VERTEX_BIT, nullptr);
	bindings[1] = getDescriptorSetLayoutBinding(1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, textureImages.size(), VK_SHADER_STAGE_FRAGMENT_BIT, nullptr);

	createVkDescriptorPool(context->device, bindings, renderer->length, &descriptorPool);
	createVkDescriptorSetLayout(context->device, bindings, &descriptorSetLayout);

	std::vector<VkDescriptorSetLayout> layouts(renderer->length);
	for (auto& layout : layouts)
		layout = this->descriptorSetLayout;

	allocateVkDescriptorSets(context->device, descriptorPool, layouts, &descriptorSets);

	uniformBuffers.resize(renderer->length);

	for (int i = 0; i < renderer->length; i++)
	{
		VkDescriptorBufferInfo bufferInfo = {};
		bufferInfo.buffer = uniformBuffers[i];
		bufferInfo.offset = 0;
		bufferInfo.range = sizeof(UBO);

		std::vector<VkWriteDescriptorSet> descriptorWrites(2);

		descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptorWrites[0].dstSet = descriptorSets[i];
		descriptorWrites[0].dstBinding = 0;
		descriptorWrites[0].dstArrayElement = 0;
		descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		descriptorWrites[0].descriptorCount = 1;
		descriptorWrites[0].pBufferInfo = &bufferInfo;
		descriptorWrites[0].pImageInfo = nullptr;
		descriptorWrites[0].pTexelBufferView = nullptr;

		std::vector<VkDescriptorImageInfo> imageInfos(textureImages.size());
		for (int j = 0; j < imageInfos.size(); j++)
		{
			imageInfos[j].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
			imageInfos[j].imageView = textureImageViews[j];
			imageInfos[j].sampler = textureSampler;
		}

		descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptorWrites[1].dstSet = descriptorSets[i];
		descriptorWrites[1].dstBinding = 1;
		descriptorWrites[1].dstArrayElement = 0;
		descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		descriptorWrites[1].descriptorCount = imageInfos.size();
		descriptorWrites[1].pBufferInfo = nullptr;
		descriptorWrites[1].pImageInfo = imageInfos.data();
		descriptorWrites[1].pTexelBufferView = nullptr;

		vkUpdateDescriptorSets(context->device, descriptorWrites.size(), descriptorWrites.data(), 0, nullptr);
	}
}

void Scene3D::createPipeline()
{
	VkPipelineShaderStageCreateInfo vertShaderStageInfo = {};
	vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
	vertShaderStageInfo.module = vertexShader;
	vertShaderStageInfo.pName = "main";

	VkPipelineShaderStageCreateInfo fragShaderStageInfo = {};
	fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
	fragShaderStageInfo.module = fragmentShader;
	fragShaderStageInfo.pName = "main";

	std::vector<VkPipelineShaderStageCreateInfo> shaderStages = {vertShaderStageInfo, fragShaderStageInfo};

	std::vector<VkVertexInputBindingDescription> binding;
	std::vector<VkVertexInputAttributeDescription> attribute;

	getVertexInputDescriptions(&binding, &attribute);

	VkPipelineVertexInputStateCreateInfo vertexInputInfo = {};
	vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	vertexInputInfo.vertexBindingDescriptionCount = binding.size();
	vertexInputInfo.pVertexBindingDescriptions = binding.data();
	vertexInputInfo.vertexAttributeDescriptionCount = attribute.size();
	vertexInputInfo.pVertexAttributeDescriptions = attribute.data();

	VkPipelineInputAssemblyStateCreateInfo inputAssembly = {};
	inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	inputAssembly.primitiveRestartEnable = VK_FALSE;

	VkViewport viewport = {};
	viewport.x = 0.0f;
	viewport.y = 0.0f;
	viewport.width = renderer->extent.width;
	viewport.height = renderer->extent.height;
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;

	VkRect2D scissor = {};
	scissor.offset.x = 0;
	scissor.offset.y = 0;
	scissor.extent.width = renderer->extent.width;
	scissor.extent.height = renderer->extent.height;

	VkPipelineViewportStateCreateInfo viewportState = {};
	viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	viewportState.viewportCount = 1;
	viewportState.pViewports = &viewport;
	viewportState.scissorCount = 1;
	viewportState.pScissors = &scissor;

	VkPipelineRasterizationStateCreateInfo rasterizer = {};
	rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	rasterizer.depthClampEnable = VK_FALSE; // Enable for VR
	rasterizer.rasterizerDiscardEnable = VK_FALSE;
	rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
	rasterizer.lineWidth = 1.0f;
	rasterizer.cullMode = VK_CULL_MODE_NONE;
	rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;
	rasterizer.depthBiasEnable = VK_FALSE;
	rasterizer.depthBiasConstantFactor = 0.0f;
	rasterizer.depthBiasClamp = 0.0f;
	rasterizer.depthBiasSlopeFactor = 0.0f;

	VkPipelineMultisampleStateCreateInfo multisampling = {};
	multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	multisampling.sampleShadingEnable = VK_FALSE;
	multisampling.rasterizationSamples = SAMPLE_COUNT;
	multisampling.minSampleShading = 1.0f;
	multisampling.pSampleMask = nullptr;
	multisampling.alphaToCoverageEnable = VK_FALSE;
	multisampling.alphaToOneEnable = VK_FALSE;

	VkPipelineDepthStencilStateCreateInfo depthStencil = {};
	depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
	depthStencil.depthTestEnable = VK_TRUE;
	depthStencil.depthWriteEnable = VK_TRUE;
	depthStencil.depthCompareOp = VK_COMPARE_OP_LESS;
	depthStencil.depthBoundsTestEnable = VK_FALSE;
	depthStencil.minDepthBounds = 0.0f;
	depthStencil.maxDepthBounds = 1.0f;
	depthStencil.stencilTestEnable = VK_FALSE;
	depthStencil.front = {};
	depthStencil.back = {};

	VkPipelineColorBlendAttachmentState colorBlendAttachment = {};
	colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT |
	                                      VK_COLOR_COMPONENT_G_BIT |
	                                      VK_COLOR_COMPONENT_B_BIT |
	                                      VK_COLOR_COMPONENT_A_BIT;
	colorBlendAttachment.blendEnable = VK_FALSE;
	colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
	colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;
	colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
	colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
	colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
	colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;

	VkPipelineColorBlendStateCreateInfo colorBlending = {};
	colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	colorBlending.logicOpEnable = VK_FALSE;
	colorBlending.logicOp = VK_LOGIC_OP_COPY;
	colorBlending.attachmentCount = 1;
	colorBlending.pAttachments = &colorBlendAttachment;
	colorBlending.blendConstants[0] = 0.0f;
	colorBlending.blendConstants[1] = 0.0f;
	colorBlending.blendConstants[2] = 0.0f;
	colorBlending.blendConstants[3] = 0.0f;

	VkPipelineLayoutCreateInfo pipelineLayoutInfo = {};
	pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipelineLayoutInfo.setLayoutCount = 1;
	pipelineLayoutInfo.pSetLayouts = &descriptorSetLayout;
	pipelineLayoutInfo.pushConstantRangeCount = 0;
	pipelineLayoutInfo.pPushConstantRanges = nullptr;

	int result = vkCreatePipelineLayout(context->device, &pipelineLayoutInfo, nullptr, &pipelineLayout);
	if (result != VK_SUCCESS)
	{
		PANIC("SCENE3D - Failed to create pipeline layout %d", result);
	}

	VkGraphicsPipelineCreateInfo pipelineInfo = {};
	pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	pipelineInfo.stageCount = shaderStages.size();
	pipelineInfo.pStages = shaderStages.data();
	pipelineInfo.pVertexInputState = &vertexInputInfo;
	pipelineInfo.pInputAssemblyState = &inputAssembly;
	pipelineInfo.pViewportState = &viewportState;
	pipelineInfo.pRasterizationState = &rasterizer;
	pipelineInfo.pMultisampleState = &multisampling;
	pipelineInfo.pDepthStencilState = &depthStencil;
	pipelineInfo.pColorBlendState = &colorBlending;
	pipelineInfo.pDynamicState = nullptr;
	pipelineInfo.layout = pipelineLayout;
	pipelineInfo.renderPass = renderer->renderPass;
	pipelineInfo.subpass = 0;
	pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;
	pipelineInfo.basePipelineIndex = -1;

	result = vkCreateGraphicsPipelines(context->device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &graphicsPipeline);
	if(result != VK_SUCCESS)
	{
		PANIC("SCENE3D - Failed to create graphics pipeline %d", result);
	}
}

void Scene3D::recordCommands()
{
	commandBuffers.resize(renderer->length);
	allocateVkCommandBuffers(context->device, nullptr, context->graphicsCommandPool,
	                         VK_COMMAND_BUFFER_LEVEL_PRIMARY, renderer->length, commandBuffers.data());

	for (size_t i = 0; i < renderer->length; i++)
	{
		VkCommandBufferBeginInfo beginInfo = {};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		beginInfo.flags = 0;
		beginInfo.pInheritanceInfo = nullptr;

		int result = vkBeginCommandBuffer(commandBuffers[i], &beginInfo);
		if (result != VK_SUCCESS)
		{
			PANIC("SCENE3D - Failed to begin recording graphics command buffer! %d", result);
		}

		VkClearValue clearColors[3];
		clearColors[0].color = {0.0f, 0.0f, 0.0f, 1.0f};
		clearColors[1].depthStencil = {1.0f, 0};
		clearColors[2].color = {0.0f, 0.0f, 0.0f, 1.0f};

		VkRenderPassBeginInfo renderPassInfo = {};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderPassInfo.renderPass = renderer->renderPass;
		renderPassInfo.framebuffer = renderer->framebuffers[i];
		renderPassInfo.renderArea.offset = {0, 0};
		renderPassInfo.renderArea.extent.width = renderer->extent.width;
		renderPassInfo.renderArea.extent.height = renderer->extent.height;
		renderPassInfo.clearValueCount = 3;
		renderPassInfo.pClearValues = clearColors;

		vkCmdBindPipeline(commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipeline);
		vkCmdBeginRenderPass(commandBuffers[i], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

		vkCmdBindDescriptorSets(commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 1, &descriptorSets[i], 0, nullptr);

		for (auto& mesh : meshes)
		{
			std::vector<VkBuffer> vertexBuffers = {mesh.vertexBuffer, mesh.instanceBuffer};
			std::vector<VkDeviceSize> offsets = {0, 0};
			vkCmdBindVertexBuffers(commandBuffers[i], 0, vertexBuffers.size(), vertexBuffers.data(), offsets.data());
			vkCmdBindIndexBuffer(commandBuffers[i], mesh.indexBuffer, 0, VK_INDEX_TYPE_UINT32);
			vkCmdDrawIndexed(commandBuffers[i], mesh.indices.size(), mesh.instances.size(), 0, 0, 0);
		}

		vkCmdEndRenderPass(commandBuffers[i]);

		result = vkEndCommandBuffer(commandBuffers[i]);
		if (result != VK_SUCCESS)
		{
			PANIC("SCENE3D - Failed to record graphics command buffer %d", result);
		}
	}
}
