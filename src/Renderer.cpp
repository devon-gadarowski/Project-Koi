#include <render/Renderer.h>

VkFormat chooseColorFormat(VkPhysicalDevice physicalDevice)
{
    VkFormat colorFormat;
    std::vector<VkFormat> colorCandidates = {VK_FORMAT_B8G8R8A8_UNORM, VK_FORMAT_R8G8B8A8_UNORM};

    for (auto& colorCandidate : colorCandidates)
    {
        VkFormatProperties props = {};
        vkGetPhysicalDeviceFormatProperties(physicalDevice, colorCandidate, &props);

        if (props.linearTilingFeatures | props.optimalTilingFeatures |props.bufferFeatures != 0)
        {
            colorFormat = colorCandidate;
            break;
        }
    }

    VALIDATE(colorFormat != VK_FORMAT_UNDEFINED, "RENDER_FRAMEWORK - Failed to find valid color image format");

    return colorFormat;
}

VkFormat chooseDepthFormat(VkPhysicalDevice physicalDevice)
{
    VkFormat depthFormat;

    std::vector<VkFormat> depthCandidates = {VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT};

    for (auto& depthCandidate : depthCandidates)
    {
        VkFormatProperties props = {};
        vkGetPhysicalDeviceFormatProperties(physicalDevice, depthCandidate, &props);

        if (props.linearTilingFeatures | props.optimalTilingFeatures | props.bufferFeatures != 0)
        {
            depthFormat = depthCandidate;
            break;
        }
    }

    VALIDATE(depthFormat != VK_FORMAT_UNDEFINED, "RENDER_FRAMEWORK - Failed to find valid depth image format");

    return depthFormat;
}

VkViewport Renderer::getDefaultVkViewport()
{
	VkViewport viewport = {};
	viewport.x = 0.0f;
	viewport.y = 0.0f;
	viewport.width = this->extent.width;
	viewport.height = this->extent.height;
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;

    return viewport;
}

VkRect2D Renderer::getDefaultScissor()
{
	VkRect2D scissor = {};
	scissor.offset.x = 0;
	scissor.offset.y = 0;
	scissor.extent.width = this->extent.width;
	scissor.extent.height = this->extent.height;

    return scissor;
}

VkPipelineRasterizationStateCreateInfo Renderer::getDefaultRasterizer()
{
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

    return rasterizer;
}

VkPipelineMultisampleStateCreateInfo Renderer::getDefaultMultisampling()
{
	VkPipelineMultisampleStateCreateInfo multisampling = {};
	multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	multisampling.sampleShadingEnable = VK_FALSE;
	multisampling.rasterizationSamples = this->sample_count;
	multisampling.minSampleShading = 1.0f;
	multisampling.pSampleMask = nullptr;
	multisampling.alphaToCoverageEnable = VK_FALSE;
	multisampling.alphaToOneEnable = VK_FALSE;

    return multisampling;
}

VkPipelineDepthStencilStateCreateInfo Renderer::getDefaultDepthStencil()
{
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

    return depthStencil;
}

void Renderer::getDefaultColorBlendAttachments(std::vector<VkPipelineColorBlendAttachmentState> & colorBlendAttachments)
{

	VkPipelineColorBlendAttachmentState colorBlendAttachment = {};
	colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
	colorBlendAttachment.blendEnable = VK_TRUE;
	colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
	colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
	colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
	colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
	colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
	colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;

	colorBlendAttachments.push_back(colorBlendAttachment);
}

VkPipelineColorBlendStateCreateInfo Renderer::getDefaultColorBlend(std::vector<VkPipelineColorBlendAttachmentState> & colorBlendAttachments)
{
	VkPipelineColorBlendStateCreateInfo colorBlending = {};
	colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	colorBlending.pNext = nullptr;
	colorBlending.flags = 0;
	colorBlending.logicOpEnable = VK_FALSE;
	colorBlending.logicOp = VK_LOGIC_OP_COPY;
	colorBlending.attachmentCount = colorBlendAttachments.size();
	colorBlending.pAttachments = colorBlendAttachments.data();
	colorBlending.blendConstants[0] = 0.0f;
	colorBlending.blendConstants[1] = 0.0f;
	colorBlending.blendConstants[2] = 0.0f;
	colorBlending.blendConstants[3] = 0.0f;

    return colorBlending;
}