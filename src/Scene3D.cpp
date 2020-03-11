#include <render/Scene3D.h>
#include <render/Utilities.h>

#include <cstring>

void Scene3D::init()
{
    app->registerSystem(&camera);
    //this->models.push_back(new TexturedModel("ada1.obj", "assets/meshes/ada1/", context, renderer, this));

    setMessageCallback(SetLighting, (message_method_t) &Scene3D::updateLighting);
    setMessageCallback(GetModelData, (message_method_t) &Scene3D::getModelData);
    setMessageCallback(AddModel, (message_method_t) &Scene3D::addModel);
}

void Scene3D::update(long elapsedTime)
{
    //this->camera.updateBuffer();
}

void Scene3D::draw(VkCommandBuffer commandbuffer)
{
    VkClearValue clearColors[3];
    clearColors[0].color = {0.0f, 0.0f, 0.0f, 1.0f};
    clearColors[1].depthStencil = {1.0f, 0};
    clearColors[2].color = {0.0f, 0.0f, 0.0f, 1.0f};

    VkRenderPassBeginInfo beginInfo = {};
    beginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    beginInfo.pNext = nullptr;
    beginInfo.renderPass = this->renderPass;
    beginInfo.framebuffer = this->framebuffers[renderer->currentImageIndex];
    beginInfo.renderArea.offset = {0, 0};
    beginInfo.renderArea.extent = renderer->extent;
    beginInfo.clearValueCount = 3;
    beginInfo.pClearValues = clearColors;

    vkCmdBeginRenderPass(commandbuffer, &beginInfo, VK_SUBPASS_CONTENTS_INLINE);

    for (auto& model : models)
    {
        model->draw(commandbuffer);
    }

    vkCmdEndRenderPass(commandbuffer);
}

Scene3D::Scene3D(Context * context, Renderer * renderer)
{
    this->context = context;
    this->renderer = renderer;

    // ===== Create Uniform Buffers =====

    camera.extent = renderer->extent;

    camera.buffers.resize(2);
	for (int i = 0; i < camera.buffers.size(); i++)
	{
		createBuffer(context, sizeof(CameraData), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                     VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                     &camera.buffers[i].buffer, &camera.buffers[i].memory);

        vkMapMemory(context->device, camera.buffers[i].memory, 0, sizeof(CameraData), 0, &camera.buffers[i].data);
        memcpy(camera.buffers[i].data, &camera.data, sizeof(CameraData));
	}

    light.buffers.resize(2);
	for (int i = 0; i < light.buffers.size(); i++)
	{
		createBuffer(context, sizeof(DirectionalLightData), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                     VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                     &light.buffers[i].buffer, &light.buffers[i].memory);

        vkMapMemory(context->device, light.buffers[i].memory, 0, sizeof(DirectionalLightData), 0, &light.buffers[i].data);
        memcpy(light.buffers[i].data, &light.data, sizeof(DirectionalLightData));
	}   

    // ===== Create VkDescriptorPool =====

    VkDescriptorPoolSize poolSize = {};
    poolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    poolSize.descriptorCount = renderer->length * 2;

    VkDescriptorPoolCreateInfo poolInfo = {};
    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.pNext = nullptr;
    poolInfo.flags = 0;
    poolInfo.maxSets = renderer->length;
    poolInfo.poolSizeCount = 1;
    poolInfo.pPoolSizes = &poolSize;

    int result = vkCreateDescriptorPool(context->device, &poolInfo, nullptr, &this->descriptorPool);
    VALIDATE(result == VK_SUCCESS, "Failed to create VkDescriptorPool %d", result);

    // ===== Create VkDescriptorSets =====

    std::vector<VkDescriptorSetLayoutBinding> bindings(2);
    bindings[0] = camera.getVkDescriptorSetLayoutBinding(0);
    bindings[1] = light.getVkDescriptorSetLayoutBinding(1);

    VkDescriptorSetLayoutCreateInfo layoutInfo = {};
    layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutInfo.pNext = nullptr;
    layoutInfo.flags = 0;
    layoutInfo.bindingCount = bindings.size();
    layoutInfo.pBindings = bindings.data();

    result = vkCreateDescriptorSetLayout(context->device, &layoutInfo, nullptr, &this->descriptorSetLayout);
    VALIDATE(result == VK_SUCCESS, "Failed to create VkDescriptorSetLayout %d", result);

    std::vector<VkDescriptorSetLayout> layouts(renderer->length, this->descriptorSetLayout);

    VkDescriptorSetAllocateInfo allocInfo = {};
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.pNext = nullptr;
    allocInfo.descriptorPool = this->descriptorPool;
    allocInfo.descriptorSetCount = layouts.size();
    allocInfo.pSetLayouts = layouts.data();

    this->descriptorSets.resize(renderer->length);
    result = vkAllocateDescriptorSets(context->device, &allocInfo, this->descriptorSets.data());
    VALIDATE(result == VK_SUCCESS, "Failed to allocate VkDescriptorSets %d", result);

    std::vector<VkWriteDescriptorSet> descriptorWrites(renderer->length * 2);

    for (uint32_t i = 0; i < this->descriptorSets.size(); i++)
    {
        VkDescriptorBufferInfo bufferInfo1 = {};
        bufferInfo1.buffer = camera.buffers[i % camera.buffers.size()].buffer;
        bufferInfo1.offset = 0;
        bufferInfo1.range = sizeof(CameraData);

        VkDescriptorBufferInfo bufferInfo2 = {};
        bufferInfo2.buffer = light.buffers[i % light.buffers.size()].buffer;
        bufferInfo2.offset = 0;
        bufferInfo2.range = sizeof(DirectionalLightData);

        descriptorWrites[2*i].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrites[2*i].dstSet = this->descriptorSets[i];
        descriptorWrites[2*i].dstBinding = 0;
        descriptorWrites[2*i].dstArrayElement = 0;
        descriptorWrites[2*i].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        descriptorWrites[2*i].descriptorCount = 1;
        descriptorWrites[2*i].pBufferInfo = &bufferInfo1;
        descriptorWrites[2*i].pImageInfo = nullptr;
        descriptorWrites[2*i].pTexelBufferView = nullptr;

            
        descriptorWrites[2*i+1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrites[2*i+1].dstSet = this->descriptorSets[i];
        descriptorWrites[2*i+1].dstBinding = 1;
        descriptorWrites[2*i+1].dstArrayElement = 0;
        descriptorWrites[2*i+1].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        descriptorWrites[2*i+1].descriptorCount = 1;
        descriptorWrites[2*i+1].pBufferInfo = &bufferInfo2;
        descriptorWrites[2*i+1].pImageInfo = nullptr;
        descriptorWrites[2*i+1].pTexelBufferView = nullptr;
    }

    vkUpdateDescriptorSets(context->device, descriptorWrites.size(), descriptorWrites.data(), 0, nullptr);

    // ===== Create VkRenderPass =====

    this->renderPass = createVkRenderPass(context->device, renderer->colorFormat, renderer->depthFormat, renderer->sample_count);

    // ===== Create VkFramebuffers =====

    framebuffers.resize(renderer->length);

    for (uint32_t i = 0; i < framebuffers.size(); i++)
    {
        framebuffers[i] = createVkFramebuffer(context->device, nullptr, 0, this->renderPass, renderer->colorImageView, renderer->depthImageView, renderer->imageviews[i], renderer->extent.width, renderer->extent.height, 1);
    }

    DEBUG("SCENE3D - Scene Created");
}

Scene3D::~Scene3D()
{
    // TODO: Cleanup

    for (int i = 0; i < camera.buffers.size(); i++)
    {
        vkUnmapMemory(context->device, camera.buffers[i].memory);
        vkDestroyBuffer(context->device, camera.buffers[i].buffer, nullptr);
        vkFreeMemory(context->device, camera.buffers[i].memory, nullptr);
    }

    for (int i = 0; i < light.buffers.size(); i++)
    {
        vkUnmapMemory(context->device, light.buffers[i].memory);
        vkDestroyBuffer(context->device, light.buffers[i].buffer, nullptr);
        vkFreeMemory(context->device, light.buffers[i].memory, nullptr);
    }

    for (auto & framebuffer : this->framebuffers)
        vkDestroyFramebuffer(context->device, framebuffer, nullptr);

    vkDestroyRenderPass(context->device, this->renderPass, nullptr);

    vkDestroyDescriptorSetLayout(context->device, descriptorSetLayout, nullptr);
    vkDestroyDescriptorPool(context->device, descriptorPool, nullptr);

    for (auto& model : models)
        delete model;

    DEBUG("SCENE3D - Scene Destroyed");
}

VkDescriptorSetLayoutBinding Camera::getVkDescriptorSetLayoutBinding(uint32_t binding)
{
    VkDescriptorSetLayoutBinding layoutBinding = {};
    layoutBinding.binding = binding;
    layoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    layoutBinding.descriptorCount = 1;
    layoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
    layoutBinding.pImmutableSamplers = nullptr;

    return layoutBinding;
}

VkDescriptorSetLayoutBinding DirectionalLight::getVkDescriptorSetLayoutBinding(uint32_t binding)
{
    VkDescriptorSetLayoutBinding layoutBinding = {};
    layoutBinding.binding = binding;
    layoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    layoutBinding.descriptorCount = 1;
    layoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
    layoutBinding.pImmutableSamplers = nullptr;

    return layoutBinding;
}

void Scene3D::updateLighting(Message * msg)
{
    DirectionalLightData * data = (DirectionalLightData *) (dynamic_cast<PointerMessage *> (msg))->data;

    this->light.data = *data;

    memcpy(this->light.buffers[renderer->currentImageIndex % this->light.buffers.size()].data, &this->light.data, sizeof(DirectionalLightData));
}

void Scene3D::addModel(Message * msg)
{
    std::vector<std::string> * args = (std::vector<std::string> *) (dynamic_cast<PointerMessage *> (msg))->data;

    try
    {
        this->models.push_back(new TexturedModel((*args)[1], (*args)[2], context, renderer, this));
    }
    catch(const std::exception& e)
    {
        WARN("Failed to load model %s", (*args)[1].c_str());
    }
    
}

void Scene3D::getModelData(Message * msg)
{
    std::vector<ModelData> * models = (std::vector<ModelData> *) (dynamic_cast<PointerMessage *> (msg))->data;

    for (int i = 0; i < this->models.size(); i++)
    {
        ModelData m = {(uint32_t) i, this->models[i]->name, (uint32_t) this->models[i]->instances.size()};
        models->push_back(m);
    }
}