#include <cstring>

#include <render/Utilities.h>
#include <render/Model.h>

uint32_t Texture::count;
VkSampler Texture::sampler;

uint32_t Model::count;
VkPipelineLayout Model::pipelineLayout;
VkPipeline Model::pipeline;

uint32_t TexturedModel::count;
VkPipelineLayout TexturedModel::pipelineLayout;
VkPipeline TexturedModel::pipeline;

void Vertex::getAttributeDescriptions(uint32_t binding, std::vector<VkVertexInputAttributeDescription> & attribDesc)
{
	attribDesc.emplace_back();
	attribDesc.back().location = attribDesc.size() - 1;
	attribDesc.back().binding = binding;
	attribDesc.back().format = VK_FORMAT_R32G32B32_SFLOAT;
	attribDesc.back().offset = offsetof(VertexData, position);

    attribDesc.emplace_back();
	attribDesc.back().location = attribDesc.size() - 1;
	attribDesc.back().binding = binding;
	attribDesc.back().format = VK_FORMAT_R32G32B32_SFLOAT;
	attribDesc.back().offset = offsetof(VertexData, color);

    attribDesc.emplace_back();
	attribDesc.back().location = attribDesc.size() - 1;
	attribDesc.back().binding = binding;
	attribDesc.back().format = VK_FORMAT_R32G32B32_SFLOAT;
	attribDesc.back().offset = offsetof(VertexData, normal);

	attribDesc.emplace_back();
	attribDesc.back().location = attribDesc.size() - 1;
	attribDesc.back().binding = binding;
	attribDesc.back().format = VK_FORMAT_R32G32_SFLOAT;
	attribDesc.back().offset = offsetof(VertexData, tex);
}

void Instance::getAttributeDescriptions(uint32_t binding, std::vector<VkVertexInputAttributeDescription> & attribDesc)
{
	attribDesc.emplace_back();
	attribDesc.back().location = attribDesc.size() - 1;
	attribDesc.back().binding = binding;
	attribDesc.back().format = VK_FORMAT_R32G32B32A32_SFLOAT;
	attribDesc.back().offset = sizeof(Vec4) * 0;

	attribDesc.emplace_back();
	attribDesc.back().location = attribDesc.size() - 1;
	attribDesc.back().binding = binding;
	attribDesc.back().format = VK_FORMAT_R32G32B32A32_SFLOAT;
	attribDesc.back().offset = sizeof(Vec4) * 1;

	attribDesc.emplace_back();
	attribDesc.back().location = attribDesc.size() - 1;
	attribDesc.back().binding = binding;
	attribDesc.back().format = VK_FORMAT_R32G32B32A32_SFLOAT;
	attribDesc.back().offset = sizeof(Vec4) * 2;

	attribDesc.emplace_back();
	attribDesc.back().location = attribDesc.size() - 1;
	attribDesc.back().binding = binding;
	attribDesc.back().format = VK_FORMAT_R32G32B32A32_SFLOAT;
	attribDesc.back().offset = sizeof(Vec4) * 3;
}

VkDescriptorSetLayoutBinding Material::getVkDescriptorSetLayoutBinding(uint32_t binding)
{
    VkDescriptorSetLayoutBinding layoutBinding = {};
    layoutBinding.binding = binding;
    layoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    layoutBinding.descriptorCount = 1;
    layoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
    layoutBinding.pImmutableSamplers = nullptr;

    return layoutBinding;
}

VkDescriptorSetLayoutBinding Texture::getVkDescriptorSetLayoutBinding(uint32_t binding)
{
    VkDescriptorSetLayoutBinding layoutBinding = {};
    layoutBinding.binding = binding;
    layoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    layoutBinding.descriptorCount = 1;
    layoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
    layoutBinding.pImmutableSamplers = nullptr;

    return layoutBinding;
}

ModelBase::ModelBase(Context * context, Renderer * renderer, Scene * scene)
{
	this->context = context;
	this->renderer = renderer;
	this->scene = scene;
}

ModelBase::~ModelBase()
{
    for (auto & shape : shapes)
	{
        vkDestroyDescriptorSetLayout(context->device, shape.descriptorSetLayout, nullptr);
		vkDestroyBuffer(context->device, shape.vertexBuffer, nullptr);
		vkFreeMemory(context->device, shape.vertexMemory, nullptr);
		vkDestroyBuffer(context->device, shape.indexBuffer, nullptr);
		vkFreeMemory(context->device, shape.indexMemory, nullptr);
	}

	for (auto & material : materials)
	{
		for (auto & buffer : material.buffers)
		{
			vkDestroyBuffer(context->device, buffer.buffer, nullptr);
			vkFreeMemory(context->device, buffer.memory, nullptr);			
		}
	}

	vkDestroyBuffer(context->device, this->instanceBuffer, nullptr);
	vkFreeMemory(context->device, this->instanceMemory, nullptr);

    vkDestroyDescriptorPool(context->device, this->descriptorPool, nullptr);
}

Model::Model(std::string filename, std::string location, Context * context, Renderer * renderer, Scene * scene) : ModelBase(context, renderer, scene)
{
    // ===== Load Model Data =====

	this->name = filename;

	loadOBJ(filename, location, context, renderer, this, nullptr);

	this->instances.resize(1);

    // ===== Create VkDescriptorPool =====

    VkDescriptorPoolSize poolSize;
    poolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    poolSize.descriptorCount = renderer->length * shapes.size();

    VkDescriptorPoolCreateInfo poolInfo = {};
    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.pNext = nullptr;
    poolInfo.flags = 0;
    poolInfo.maxSets = renderer->length * shapes.size();
    poolInfo.poolSizeCount = 1;
    poolInfo.pPoolSizes = &poolSize;

    int result = vkCreateDescriptorPool(context->device, &poolInfo, nullptr, &this->descriptorPool);
    VALIDATE(result == VK_SUCCESS, "Failed to create VkDescriptorPool %d", result);

    // ===== Create VkDescriptorSets (per shape) =====

    for (auto& shape : shapes)
    {
        Material & m = materials[shape.materialID];

	    VkDescriptorSetLayoutBinding binding = m.getVkDescriptorSetLayoutBinding(0);

        VkDescriptorSetLayoutCreateInfo layoutInfo = {};
        layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        layoutInfo.pNext = nullptr;
        layoutInfo.flags = 0;
        layoutInfo.bindingCount = 1;
        layoutInfo.pBindings = &binding;

        result = vkCreateDescriptorSetLayout(context->device, &layoutInfo, nullptr, &shape.descriptorSetLayout);
        VALIDATE(result == VK_SUCCESS, "Failed to create VkDescriptorSetLayout %d", result);

        std::vector<VkDescriptorSetLayout> layouts(renderer->length, shape.descriptorSetLayout);

        VkDescriptorSetAllocateInfo allocInfo = {};
        allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        allocInfo.pNext = nullptr;
        allocInfo.descriptorPool = this->descriptorPool;
        allocInfo.descriptorSetCount = layouts.size();
        allocInfo.pSetLayouts = layouts.data();

        shape.descriptorSets.resize(renderer->length);
        result = vkAllocateDescriptorSets(context->device, &allocInfo, shape.descriptorSets.data());
        VALIDATE(result == VK_SUCCESS, "Failed to allocate VkDescriptorSets %d", result);

        std::vector<VkWriteDescriptorSet> descriptorWrites(renderer->length);

        for (uint32_t i = 0; i < descriptorWrites.size(); i++)
        {
            VkDescriptorBufferInfo bufferInfo = {};
            bufferInfo.buffer = m.buffers[i].buffer;
            bufferInfo.offset = 0;
            bufferInfo.range = sizeof(MaterialData);

            descriptorWrites[i].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            descriptorWrites[i].dstSet = shape.descriptorSets[i];
            descriptorWrites[i].dstBinding = 0;
            descriptorWrites[i].dstArrayElement = 0;
            descriptorWrites[i].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
            descriptorWrites[i].descriptorCount = 1;
            descriptorWrites[i].pBufferInfo = &bufferInfo;
            descriptorWrites[i].pImageInfo = nullptr;
            descriptorWrites[i].pTexelBufferView = nullptr;
        }

        vkUpdateDescriptorSets(context->device, descriptorWrites.size(), descriptorWrites.data(), 0, nullptr);
    }

    // ===== Create Pipeline (On First Instantiation) =====

    if (Model::count == 0)
        this->createVkPipeline();

    Model::count++;
}

Model::~Model()
{
    Model::count--;

    if (Model::count == 0)
    {
        vkDestroyPipelineLayout(context->device, Model::pipelineLayout, nullptr);
        vkDestroyPipeline(context->device, Model::pipeline, nullptr);
    }
}

void Model::draw(VkCommandBuffer commandbuffer)
{
	for (auto& shape : shapes)
	{
		VkDescriptorSet descriptors[] = {scene->descriptorSets[renderer->currentImageIndex], shape.descriptorSets[renderer->currentImageIndex]};
		VkBuffer vertexBuffers[] = {shape.vertexBuffer, instanceBuffer};
		VkDeviceSize offsets[] = {0, 0};

        vkCmdBindPipeline(commandbuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, Model::pipeline);
		vkCmdBindDescriptorSets(commandbuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 2, descriptors, 0, nullptr);
		vkCmdBindVertexBuffers(commandbuffer, 0, 2, vertexBuffers, offsets);
		vkCmdBindIndexBuffer(commandbuffer, shape.indexBuffer, 0, VK_INDEX_TYPE_UINT32);
		vkCmdDrawIndexed(commandbuffer, shape.indices.size(), instances.size(), 0, 0, 0);
	}
}

void Model::createVkPipeline()
{
    // ===== Pipeline Layout =====

    std::vector<VkDescriptorSetLayout> descriptorSetLayouts;

    for (auto& shape : shapes)
        descriptorSetLayouts.push_back(shape.descriptorSetLayout);

    VkPipelineLayoutCreateInfo createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	createInfo.pNext = nullptr;
	createInfo.flags = 0;
	createInfo.setLayoutCount = descriptorSetLayouts.size();
	createInfo.pSetLayouts = descriptorSetLayouts.data();
	createInfo.pushConstantRangeCount = 0;
	createInfo.pPushConstantRanges = nullptr;

	VkPipelineLayout layout;
	int result = vkCreatePipelineLayout(context->device, &createInfo, nullptr, &Model::pipelineLayout);
	VALIDATE(result == VK_SUCCESS, "Failed to create pipeline layout %d", result);

	// ===== Pipeline Shaders =====

	std::vector<VkPipelineShaderStageCreateInfo> shaderStages(2);

	VkShaderModule vertexShader = loadShader(context, "bin/vert.spv");
	shaderStages[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	shaderStages[0].stage = VK_SHADER_STAGE_VERTEX_BIT;
	shaderStages[0].module = vertexShader;
	shaderStages[0].pName = "main";

	VkShaderModule fragmentShader = loadShader(context, "bin/frag.spv");
	shaderStages[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	shaderStages[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
	shaderStages[1].module = fragmentShader;
	shaderStages[1].pName = "main";

	// ===== Pipeline Vertex Input Attributes =====

	std::vector<VkVertexInputBindingDescription> bindingDesc(2);
	std::vector<VkVertexInputAttributeDescription> attribDesc;

	Vertex::getAttributeDescriptions(0, attribDesc);
	bindingDesc[0].binding = 0;
	bindingDesc[0].stride = sizeof(VertexData);
	bindingDesc[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

	Instance::getAttributeDescriptions(1, attribDesc);
	bindingDesc[1].binding = 1;
	bindingDesc[1].stride = sizeof(InstanceData);
	bindingDesc[1].inputRate = VK_VERTEX_INPUT_RATE_INSTANCE;

	VkPipelineVertexInputStateCreateInfo vertexInputInfo = {};
	vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	vertexInputInfo.vertexBindingDescriptionCount = bindingDesc.size();
	vertexInputInfo.pVertexBindingDescriptions = bindingDesc.data();
	vertexInputInfo.vertexAttributeDescriptionCount = attribDesc.size();
	vertexInputInfo.pVertexAttributeDescriptions = attribDesc.data();

	VkPipelineInputAssemblyStateCreateInfo inputAssembly = {};
	inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	inputAssembly.primitiveRestartEnable = VK_FALSE;

	// ===== Pipeline Viewport =====

	VkViewport viewport = renderer->getDefaultVkViewport();
	VkRect2D scissor = renderer->getDefaultScissor();

	VkPipelineViewportStateCreateInfo viewportState = {};
	viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	viewportState.viewportCount = 1;
	viewportState.pViewports = &viewport;
	viewportState.scissorCount = 1;
	viewportState.pScissors = &scissor;

	// ===== Pipeline Rasterizer =====

	VkPipelineRasterizationStateCreateInfo rasterizer = renderer->getDefaultRasterizer();

	// ===== Pipeline Multisampling =====

	VkPipelineMultisampleStateCreateInfo multisampling = renderer->getDefaultMultisampling();

	// ===== Pipeline Depth Stencil =====

	VkPipelineDepthStencilStateCreateInfo depthStencil = renderer->getDefaultDepthStencil();

	// ===== Pipeline Color Blend =====

	std::vector<VkPipelineColorBlendAttachmentState> colorBlendAttachments;
	renderer->getDefaultColorBlendAttachments(colorBlendAttachments);
	VkPipelineColorBlendStateCreateInfo colorBlending = renderer->getDefaultColorBlend(colorBlendAttachments);

	// ===== Create Pipeline! =====
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
	pipelineInfo.renderPass = scene->renderPass;
	pipelineInfo.subpass = 0;
	pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;
	pipelineInfo.basePipelineIndex = -1;

	result = vkCreateGraphicsPipelines(context->device, nullptr, 1, &pipelineInfo, nullptr, &Model::pipeline);
	VALIDATE(result == VK_SUCCESS, "Failed to create graphics pipeline %d", result);

	vkDestroyShaderModule(context->device, vertexShader, nullptr);
	vkDestroyShaderModule(context->device, fragmentShader, nullptr);
}

Texture::Texture(std::string filename, Context * context, Renderer * renderer)
{
	this->context = context;

	loadMeshTexture(filename, context, renderer, this);

	if (Texture::count == 0)
		createMeshTextureSampler(context->device, &Texture::sampler);

	Texture::count++;
}

Texture::~Texture()
{
	vkDestroyImage(context->device, image, nullptr);
	vkDestroyImageView(context->device, imageView, nullptr);
	vkFreeMemory(context->device, memory, nullptr);

	Texture::count--;
	if (Texture::count == 0)
		vkDestroySampler(context->device, Texture::sampler, nullptr);
}

TexturedModel::TexturedModel(std::string filename, std::string location, Context * context, Renderer * renderer, Scene * scene) : ModelBase(context, renderer, scene)
{
    // ===== Load Model Data =====

	this->name = filename;
	
	std::vector<std::string> texturenames;

	loadOBJ(filename, location, context, renderer, this, &texturenames);

	for (auto & texturename : texturenames)
		this->textures.push_back(new Texture(location + texturename, context, renderer));

	this->instances.resize(1);

	// ===== Create Vertex/Index/Instance/Uniform Buffers =====

	for (auto & shape : shapes)
	{
		createVertexBuffer(context, shape.vertices, &shape.vertexBuffer, &shape.vertexMemory, &shape.vertexBufferSize);
		createIndexBuffer(context, shape.indices, &shape.indexBuffer, &shape.indexMemory, &shape.indexBufferSize);
	}

	for (auto & material : materials)
	{
		material.buffers.resize(renderer->length);
		for (int i = 0; i < material.buffers.size(); i++)
		{
			createBuffer(context, sizeof(MaterialData), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
						VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
						&material.buffers[i].buffer, &material.buffers[i].memory);

			vkMapMemory(context->device, material.buffers[i].memory, 0, sizeof(MaterialData), 0, &material.buffers[i].data);
			memcpy(material.buffers[i].data, &material.data, sizeof(MaterialData));
			vkUnmapMemory(context->device, material.buffers[i].memory);
		}   
	}

	createInstanceBuffer(context, this->instances, &this->instanceBuffer, &this->instanceMemory, &this->instanceBufferSize);

    // ===== Create VkDescriptorPool =====

    std::vector<VkDescriptorPoolSize> poolSizes(2);
    poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    poolSizes[0].descriptorCount = renderer->length * shapes.size();
    poolSizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    poolSizes[1].descriptorCount = renderer->length * shapes.size();

    VkDescriptorPoolCreateInfo poolInfo = {};
    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.pNext = nullptr;
    poolInfo.flags = 0;
    poolInfo.maxSets = renderer->length * shapes.size();
    poolInfo.poolSizeCount = poolSizes.size();
    poolInfo.pPoolSizes = poolSizes.data();

    int result = vkCreateDescriptorPool(context->device, &poolInfo, nullptr, &this->descriptorPool);
    VALIDATE(result == VK_SUCCESS, "Failed to create VkDescriptorPool %d", result);

    // ===== Create VkDescriptorSets (per shape) =====

    for (auto& shape : shapes)
    {
        Material & m = materials[shape.materialID];
        Texture * t = textures[shape.materialID];

	    std::vector<VkDescriptorSetLayoutBinding> bindings(2);
        bindings[0] = m.getVkDescriptorSetLayoutBinding(0);
        bindings[1] = t->getVkDescriptorSetLayoutBinding(1);

        VkDescriptorSetLayoutCreateInfo layoutInfo = {};
        layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        layoutInfo.pNext = nullptr;
        layoutInfo.flags = 0;
        layoutInfo.bindingCount = bindings.size();
        layoutInfo.pBindings = bindings.data();

        result = vkCreateDescriptorSetLayout(context->device, &layoutInfo, nullptr, &shape.descriptorSetLayout);
        VALIDATE(result == VK_SUCCESS, "Failed to create VkDescriptorSetLayout %d", result);

        std::vector<VkDescriptorSetLayout> layouts(renderer->length, shape.descriptorSetLayout);

        VkDescriptorSetAllocateInfo allocInfo = {};
        allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        allocInfo.pNext = nullptr;
        allocInfo.descriptorPool = this->descriptorPool;
        allocInfo.descriptorSetCount = layouts.size();
        allocInfo.pSetLayouts = layouts.data();

        shape.descriptorSets.resize(renderer->length);
        result = vkAllocateDescriptorSets(context->device, &allocInfo, shape.descriptorSets.data());
        VALIDATE(result == VK_SUCCESS, "Failed to allocate VkDescriptorSets %d", result);

        std::vector<VkWriteDescriptorSet> descriptorWrites(renderer->length * 2);

        VkDescriptorImageInfo imageInfo = {};
        imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        imageInfo.imageView = t->imageView;
        imageInfo.sampler = Texture::sampler;


        for (uint32_t i = 0; i < shape.descriptorSets.size(); i++)
        {
            VkDescriptorBufferInfo bufferInfo = {};
            bufferInfo.buffer = m.buffers[i].buffer;
            bufferInfo.offset = 0;
            bufferInfo.range = sizeof(MaterialData);

            descriptorWrites[2*i].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            descriptorWrites[2*i].dstSet = shape.descriptorSets[i];
            descriptorWrites[2*i].dstBinding = 0;
            descriptorWrites[2*i].dstArrayElement = 0;
            descriptorWrites[2*i].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
            descriptorWrites[2*i].descriptorCount = 1;
            descriptorWrites[2*i].pBufferInfo = &bufferInfo;
            descriptorWrites[2*i].pImageInfo = nullptr;
            descriptorWrites[2*i].pTexelBufferView = nullptr;

            	
            descriptorWrites[2*i+1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            descriptorWrites[2*i+1].dstSet = shape.descriptorSets[i];
            descriptorWrites[2*i+1].dstBinding = 1;
            descriptorWrites[2*i+1].dstArrayElement = 0;
            descriptorWrites[2*i+1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
            descriptorWrites[2*i+1].descriptorCount = 1;
            descriptorWrites[2*i+1].pBufferInfo = nullptr;
            descriptorWrites[2*i+1].pImageInfo = &imageInfo;
            descriptorWrites[2*i+1].pTexelBufferView = nullptr;
        }

        vkUpdateDescriptorSets(context->device, descriptorWrites.size(), descriptorWrites.data(), 0, nullptr);
    }

    // ===== Create Pipeline (On First Instantiation) =====

    if (TexturedModel::count == 0)
	{
        this->createVkPipeline();
	}

	TexturedModel::count++;
}

TexturedModel::~TexturedModel()
{
    TexturedModel::count--;

	for (auto & texture : textures)
		delete texture;

    if (TexturedModel::count == 0)
    {
        vkDestroyPipelineLayout(context->device, TexturedModel::pipelineLayout, nullptr);
        vkDestroyPipeline(context->device, TexturedModel::pipeline, nullptr);
    }
}

void TexturedModel::draw(VkCommandBuffer commandbuffer)
{
	for (auto& shape : shapes)
	{
		VkDescriptorSet descriptors[] = {scene->descriptorSets[renderer->currentImageIndex], shape.descriptorSets[renderer->currentImageIndex]};
		VkBuffer vertexBuffers[] = {shape.vertexBuffer, instanceBuffer};
		VkDeviceSize offsets[] = {0, 0};

        vkCmdBindPipeline(commandbuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, TexturedModel::pipeline);
		vkCmdBindDescriptorSets(commandbuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 2, descriptors, 0, nullptr);
		vkCmdBindVertexBuffers(commandbuffer, 0, 2, vertexBuffers, offsets);
		vkCmdBindIndexBuffer(commandbuffer, shape.indexBuffer, 0, VK_INDEX_TYPE_UINT32);
		vkCmdDrawIndexed(commandbuffer, shape.indices.size(), instances.size(), 0, 0, 0);
	}
}

void TexturedModel::createVkPipeline()
{
    // ===== Pipeline Layout =====

    std::vector<VkDescriptorSetLayout> descriptorSetLayouts;

    descriptorSetLayouts.push_back(scene->descriptorSetLayout);
    descriptorSetLayouts.push_back(shapes[0].descriptorSetLayout);

    VkPipelineLayoutCreateInfo createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	createInfo.pNext = nullptr;
	createInfo.flags = 0;
	createInfo.setLayoutCount = descriptorSetLayouts.size();
	createInfo.pSetLayouts = descriptorSetLayouts.data();
	createInfo.pushConstantRangeCount = 0;
	createInfo.pPushConstantRanges = nullptr;

	VkPipelineLayout layout;
	int result = vkCreatePipelineLayout(context->device, &createInfo, nullptr, &TexturedModel::pipelineLayout);
	VALIDATE(result == VK_SUCCESS, "Failed to create pipeline layout %d", result);

	// ===== Pipeline Shaders =====

	std::vector<VkPipelineShaderStageCreateInfo> shaderStages(2);

	VkShaderModule vertexShader = loadShader(context, "bin/vert.spv");
	shaderStages[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	shaderStages[0].stage = VK_SHADER_STAGE_VERTEX_BIT;
	shaderStages[0].module = vertexShader;
	shaderStages[0].pName = "main";

	VkShaderModule fragmentShader = loadShader(context, "bin/frag.spv");
	shaderStages[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	shaderStages[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
	shaderStages[1].module = fragmentShader;
	shaderStages[1].pName = "main";

	// ===== Pipeline Vertex Input Attributes =====

	std::vector<VkVertexInputBindingDescription> bindingDesc(2);
	std::vector<VkVertexInputAttributeDescription> attribDesc;

	Vertex::getAttributeDescriptions(0, attribDesc);
	bindingDesc[0].binding = 0;
	bindingDesc[0].stride = sizeof(VertexData);
	bindingDesc[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

	Instance::getAttributeDescriptions(1, attribDesc);
	bindingDesc[1].binding = 1;
	bindingDesc[1].stride = sizeof(InstanceData);
	bindingDesc[1].inputRate = VK_VERTEX_INPUT_RATE_INSTANCE;

	VkPipelineVertexInputStateCreateInfo vertexInputInfo = {};
	vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	vertexInputInfo.vertexBindingDescriptionCount = bindingDesc.size();
	vertexInputInfo.pVertexBindingDescriptions = bindingDesc.data();
	vertexInputInfo.vertexAttributeDescriptionCount = attribDesc.size();
	vertexInputInfo.pVertexAttributeDescriptions = attribDesc.data();

	VkPipelineInputAssemblyStateCreateInfo inputAssembly = {};
	inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	inputAssembly.primitiveRestartEnable = VK_FALSE;

	// ===== Pipeline Viewport =====

	VkViewport viewport = renderer->getDefaultVkViewport();
	VkRect2D scissor = renderer->getDefaultScissor();

	VkPipelineViewportStateCreateInfo viewportState = {};
	viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	viewportState.viewportCount = 1;
	viewportState.pViewports = &viewport;
	viewportState.scissorCount = 1;
	viewportState.pScissors = &scissor;

	// ===== Pipeline Rasterizer =====

	VkPipelineRasterizationStateCreateInfo rasterizer = renderer->getDefaultRasterizer();

	// ===== Pipeline Multisampling =====

	VkPipelineMultisampleStateCreateInfo multisampling = renderer->getDefaultMultisampling();

	// ===== Pipeline Depth Stencil =====

	VkPipelineDepthStencilStateCreateInfo depthStencil = renderer->getDefaultDepthStencil();

	// ===== Pipeline Color Blend =====

	std::vector<VkPipelineColorBlendAttachmentState> colorBlendAttachments;
	renderer->getDefaultColorBlendAttachments(colorBlendAttachments);
	VkPipelineColorBlendStateCreateInfo colorBlending = renderer->getDefaultColorBlend(colorBlendAttachments);

	// ===== Create Pipeline! =====
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
	pipelineInfo.renderPass = scene->renderPass;
	pipelineInfo.subpass = 0;
	pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;
	pipelineInfo.basePipelineIndex = -1;

	result = vkCreateGraphicsPipelines(context->device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &TexturedModel::pipeline);
	VALIDATE(result == VK_SUCCESS, "Failed to create graphics pipeline %d", result);

	vkDestroyShaderModule(context->device, vertexShader, nullptr);
	vkDestroyShaderModule(context->device, fragmentShader, nullptr);
}