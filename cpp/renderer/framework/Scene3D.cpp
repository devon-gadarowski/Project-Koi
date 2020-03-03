
#include <RenderFramework.h>

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <chrono>

using namespace RenderFramework;

void Scene3D::updateUBO(uint32_t imageIndex)
{
	glm::mat4 view = glm::lookAt(
		glm::vec3(camera.position.x, camera.position.y, camera.position.z),
		glm::vec3(camera.position.x + camera.direction.x, camera.position.y + camera.direction.y, camera.position.z + camera.direction.z),
		glm::vec3(camera.up.x, camera.up.y, camera.up.z)
	);

	glm::mat4 proj = glm::perspective(glm::radians(45.0f), renderer->extent.width / (float) renderer->extent.height, 0.1f, 50.0f);

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
	vkMapMemory(context->device, uniformBuffersMemory[imageIndex], 0, sizeof(ubo), 0, &data);
	memcpy(data, &ubo, sizeof(ubo));
	vkUnmapMemory(context->device, uniformBuffersMemory[imageIndex]);
}

VkCommandBuffer Scene3D::getFrame(uint32_t imageIndex)
{
	return commandBuffers[imageIndex];
}

Scene3D::Scene3D(Context * context, Renderer * renderer, std::string filename)
{
	this->context = context;
	this->renderer = renderer;

	std::vector<std::string> diffuseTextureNames;

	RenderFramework::loadSceneModels(filename, &models);

	for (auto& model : models)
	{
		// Create Vertex/Index/Instance Buffers
		for (auto& mesh : model.shapes)
			RenderFramework::createMeshBuffers(context, &mesh);

		RenderFramework::createInstanceBuffer(context, model.instances, &model.instanceBuffer, &model.instanceMemory, &model.instanceBufferSize);

		// Load Textures
		for (auto& material : model.materials)
		{
			material.ka   = loadMeshTexture(context, renderer, &material.ambientTexture);
			material.kd   = loadMeshTexture(context, renderer, &material.diffuseTexture);
			material.ks   = loadMeshTexture(context, renderer, &material.specularTexture);
			material.norm = loadMeshTexture(context, renderer, &material.normalTexture);

			createBuffer(context, sizeof(Material) - offsetof(Material, ambient), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
			             VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
			             &material.buffer, &material.bufferMemory);
		}
	}
	// Load Shaders
	RenderFramework::createShaderModule(context, "../assets/shaders/vert.spv", &vertexShader);
	RenderFramework::createShaderModule(context, "../assets/shaders/frag.spv", &fragmentShader);

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

	DEBUG("SCENE3D - Scene Created %s", filename.c_str());
}

Scene3D::~Scene3D()
{
	vkQueueWaitIdle(context->graphicsQueue.queue);
	vkQueueWaitIdle(context->presentQueue.queue);

	vkFreeCommandBuffers(context->device, context->graphicsCommandPool, renderer->length, commandBuffers.data());

	vkDestroyPipeline(context->device, graphicsPipeline, nullptr);
	vkDestroyPipelineLayout(context->device, pipelineLayout, nullptr);

	for (int i = 0; i < uniformBuffers.size(); i++)
	{
		vkDestroyBuffer(context->device, uniformBuffers[i], nullptr);
		vkFreeMemory(context->device, uniformBuffersMemory[i], nullptr);
	}

	vkDestroyDescriptorSetLayout(context->device, descriptorSetLayout, nullptr);

	vkDestroySampler(context->device, textureSampler, nullptr);

	for (auto& model : models)
	{
		vkDestroyBuffer(context->device, model.instanceBuffer, nullptr);
		vkFreeMemory(context->device, model.instanceMemory, nullptr);

		for (auto& mesh : model.shapes)
		{
			vkDestroyBuffer(context->device, mesh.vertexBuffer, nullptr);
			vkFreeMemory(context->device, mesh.vertexMemory, nullptr);

			vkDestroyBuffer(context->device, mesh.indexBuffer, nullptr);
			vkFreeMemory(context->device, mesh.indexMemory, nullptr);
		}

		for (auto& material : model.materials)
		{
			vkDestroyDescriptorSetLayout(context->device, material.descriptorSetLayout, nullptr);

			vkDestroyBuffer(context->device, material.buffer, nullptr);
			vkFreeMemory(context->device, material.bufferMemory, nullptr);

			vkDestroyImage(context->device, material.ambientTexture.image, nullptr);
			vkFreeMemory(context->device, material.ambientTexture.memory, nullptr);
			vkDestroyImageView(context->device, material.ambientTexture.imageView, nullptr);

			vkDestroyImage(context->device, material.diffuseTexture.image, nullptr);
			vkFreeMemory(context->device, material.diffuseTexture.memory, nullptr);
			vkDestroyImageView(context->device, material.diffuseTexture.imageView, nullptr);

			vkDestroyImage(context->device, material.specularTexture.image, nullptr);
			vkFreeMemory(context->device, material.specularTexture.memory, nullptr);
			vkDestroyImageView(context->device, material.specularTexture.imageView, nullptr);

			vkDestroyImage(context->device, material.normalTexture.image, nullptr);
			vkFreeMemory(context->device, material.normalTexture.memory, nullptr);
			vkDestroyImageView(context->device, material.normalTexture.imageView, nullptr);
		}
	}

	vkDestroyDescriptorPool(context->device, descriptorPool, nullptr);

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
	std::vector<VkDescriptorSetLayoutBinding> bindings(1);

	bindings[0] = getDescriptorSetLayoutBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_VERTEX_BIT, nullptr);

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

		std::vector<VkWriteDescriptorSet> descriptorWrites(1);

		descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptorWrites[0].dstSet = descriptorSets[i];
		descriptorWrites[0].dstBinding = 0;
		descriptorWrites[0].dstArrayElement = 0;
		descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		descriptorWrites[0].descriptorCount = 1;
		descriptorWrites[0].pBufferInfo = &bufferInfo;
		descriptorWrites[0].pImageInfo = nullptr;
		descriptorWrites[0].pTexelBufferView = nullptr;

		vkUpdateDescriptorSets(context->device, descriptorWrites.size(), descriptorWrites.data(), 0, nullptr);
	}

	for (auto& model : models)
	{
		for (auto& material : model.materials)
		{
			bindings.resize(2);
			bindings[0] = getDescriptorSetLayoutBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr);
			bindings[1] = getDescriptorSetLayoutBinding(1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 4, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr);

			createVkDescriptorPool(context->device, bindings, 1, &material.descriptorPool);
			createVkDescriptorSetLayout(context->device, bindings, &material.descriptorSetLayout);

			layouts.resize(1);
			layouts[0] = material.descriptorSetLayout;

			std::vector<VkDescriptorSet> set(1);
			allocateVkDescriptorSets(context->device, material.descriptorPool, layouts, &set);
			material.descriptorSet = set[0];

			std::vector<VkDescriptorImageInfo> imageInfos;

			if (material.ka)
			{
				VkDescriptorImageInfo info = {};
				info.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
				info.imageView = material.ambientTexture.imageView;
				info.sampler = textureSampler;
				imageInfos.push_back(info);

				DEBUG("a");
			}
			if (material.kd)
			{
				VkDescriptorImageInfo info = {};
				info.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
				info.imageView = material.diffuseTexture.imageView;
				info.sampler = textureSampler;
				imageInfos.push_back(info);

				DEBUG("d");
			}
			if (material.ks)
			{
				VkDescriptorImageInfo info = {};
				info.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
				info.imageView = material.specularTexture.imageView;
				info.sampler = textureSampler;
				imageInfos.push_back(info);

				DEBUG("s");
			}
			if (material.norm)
			{
				VkDescriptorImageInfo info = {};
				info.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
				info.imageView = material.normalTexture.imageView;
				info.sampler = textureSampler;
				imageInfos.push_back(info);

				DEBUG("n");
			}
			std::vector<VkWriteDescriptorSet> descriptorWrites(2);

			VkDescriptorBufferInfo bufferInfo = {};
			bufferInfo.buffer = material.buffer;
			bufferInfo.offset = 0;
			bufferInfo.range = sizeof(Material) - offsetof(Material, ambient);

			descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			descriptorWrites[0].dstSet = material.descriptorSet;
			descriptorWrites[0].dstBinding = 0;
			descriptorWrites[0].dstArrayElement = 0;
			descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			descriptorWrites[0].descriptorCount = 1;
			descriptorWrites[0].pBufferInfo = &bufferInfo;
			descriptorWrites[0].pImageInfo = nullptr;
			descriptorWrites[0].pTexelBufferView = nullptr;

			descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			descriptorWrites[1].dstSet = material.descriptorSet;
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

	VkDescriptorSetLayout layouts[] = {descriptorSetLayout, models[0].materials[0].descriptorSetLayout};

	VkPipelineLayoutCreateInfo pipelineLayoutInfo = {};
	pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipelineLayoutInfo.setLayoutCount = 2;
	pipelineLayoutInfo.pSetLayouts = layouts;
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

		for (auto& model : models)
		{
			for (auto& mesh : model.shapes)
			{
				vkCmdBindDescriptorSets(commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 1, 1, &model.materials[mesh.materialID].descriptorSet, 0, nullptr);

				std::vector<VkBuffer> vertexBuffers = {mesh.vertexBuffer, model.instanceBuffer};
				std::vector<VkDeviceSize> offsets = {0, 0};
				vkCmdBindVertexBuffers(commandBuffers[i], 0, vertexBuffers.size(), vertexBuffers.data(), offsets.data());
				vkCmdBindIndexBuffer(commandBuffers[i], mesh.indexBuffer, 0, VK_INDEX_TYPE_UINT32);
				vkCmdDrawIndexed(commandBuffers[i], mesh.indices.size(), model.instances.size(), 0, 0, 0);
			}
		}

		vkCmdEndRenderPass(commandBuffers[i]);

		result = vkEndCommandBuffer(commandBuffers[i]);
		if (result != VK_SUCCESS)
		{
			PANIC("SCENE3D - Failed to record graphics command buffer %d", result);
		}
	}
}
