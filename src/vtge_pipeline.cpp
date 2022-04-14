#include "vtge_pipeline.hpp"
#include "vtge_vertex.hpp"
#include "vtge_getter_and_checker_functions.hpp"
#include "vtge_swapchain.hpp"
#include "vtge_graphics.hpp"

Pipeline::Pipeline(std::string vertPath, std::string fragPath, Swapchain *swapchain,
                   VkRenderPass *renderPass, VkDescriptorSetLayout *descriptorSetLayout, uint32_t setLayoutCount, uint32_t subpassIndex)
{
    // camera space stuff pipeline constructor
    this->vertFilePath = vertPath;
    this->fragFilePath = fragPath;
    // this->swapchain = swapchain;
    this->renderPass = renderPass;
    this->descriptorSetLayout = descriptorSetLayout;
    this->setLayoutCount = setLayoutCount;
    this->viewPortHeight = (float)swapchain->swapchainExtent.height;
    this->viewPortWidth = (float)swapchain->swapchainExtent.width;
    this->extent = swapchain->swapchainExtent;
    createPipeline(subpassIndex);
}

Pipeline::Pipeline(std::string vertPath, std::string fragPath, VkRenderPass *renderPass,
                   VkDescriptorSetLayout *descriptorSetLayout, uint32_t setLayoutCount, uint32_t subpassIndex)
{
    // shadow map pipeline constructor
    this->vertFilePath = vertPath;
    this->fragFilePath = fragPath;
    // this->swapchain = swapchain;
    this->renderPass = renderPass;
    this->descriptorSetLayout = descriptorSetLayout;
    this->setLayoutCount = setLayoutCount;
    this->viewPortHeight = getterChecker::getShadowMapHeight();
    this->viewPortWidth = getterChecker::getShadowMapWidth();
    this->extent.height = this->viewPortHeight;
    this->extent.width = this->viewPortWidth;
    createDepthOnlyPipeline(subpassIndex);
}

Pipeline::~Pipeline()
{
    VkDevice device = Graphics::getDevice();
    vkDestroyPipeline(device, pipeline, nullptr);
    vkDestroyPipelineLayout(device, pipelineLayout, nullptr);
}

Pipeline *Pipeline::createDrawingPipeline(std::string vertPath, std::string fragPath, Swapchain *swapchain,
                                          VkRenderPass *renderPass, VkDescriptorSetLayout *descriptorSetLayout,
                                          uint32_t setLayoutCount, uint32_t subpassIndex)
{
    Pipeline *pipe = new Pipeline(vertPath, fragPath, swapchain, renderPass, descriptorSetLayout, setLayoutCount, subpassIndex);
    return pipe;
}

Pipeline *Pipeline::createShadowPipeline(std::string vertPath, std::string fragPath, VkRenderPass *renderPass,
                                         VkDescriptorSetLayout *descriptorSetLayout, uint32_t setLayoutCount, uint32_t subpassIndex)
{
    Pipeline *pipe = new Pipeline(vertPath, fragPath, renderPass, descriptorSetLayout, setLayoutCount, subpassIndex);
    return pipe;
}
VkPipelineColorBlendStateCreateInfo Pipeline::createColorBlending()
{
    VkPipelineColorBlendAttachmentState *colorBlendAttachment = new VkPipelineColorBlendAttachmentState();
    colorBlendAttachment->colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    colorBlendAttachment->blendEnable = VK_FALSE;
    colorBlendAttachment->srcColorBlendFactor = VK_BLEND_FACTOR_ONE;  // Optional
    colorBlendAttachment->dstColorBlendFactor = VK_BLEND_FACTOR_ZERO; // Optional
    colorBlendAttachment->colorBlendOp = VK_BLEND_OP_ADD;             // Optional
    colorBlendAttachment->srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;  // Optional
    colorBlendAttachment->dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO; // Optional
    colorBlendAttachment->alphaBlendOp = VK_BLEND_OP_ADD;             // Optional
    colorBlendAttachment->blendEnable = VK_TRUE;
    colorBlendAttachment->srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
    colorBlendAttachment->dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
    colorBlendAttachment->colorBlendOp = VK_BLEND_OP_ADD;
    colorBlendAttachment->srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
    colorBlendAttachment->dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
    colorBlendAttachment->alphaBlendOp = VK_BLEND_OP_ADD;

    VkPipelineColorBlendStateCreateInfo colorBlending{};
    colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    colorBlending.logicOpEnable = VK_FALSE;
    colorBlending.logicOp = VK_LOGIC_OP_COPY; // Optional
    colorBlending.attachmentCount = 1;
    colorBlending.pAttachments = colorBlendAttachment;
    colorBlending.blendConstants[0] = 0.0f; // Optional
    colorBlending.blendConstants[1] = 0.0f; // Optional
    colorBlending.blendConstants[2] = 0.0f; // Optional
    colorBlending.blendConstants[3] = 0.0f; // Optional

    return colorBlending;
}

VkPipelineMultisampleStateCreateInfo Pipeline::createMultiSampling(VkSampleCountFlagBits sampleCount)
{
    VkPipelineMultisampleStateCreateInfo multisampling{};
    multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisampling.sampleShadingEnable = VK_FALSE; // set to VK_TRUE to enable sample shading (anti aliasing for textures)
    multisampling.rasterizationSamples = sampleCount;
    multisampling.minSampleShading = 1.0f;          // Optional min fraction for sample shading; closer to 1 the smoother
    multisampling.pSampleMask = nullptr;            // Optional
    multisampling.alphaToCoverageEnable = VK_FALSE; // Optional
    multisampling.alphaToOneEnable = VK_FALSE;      // Optional
    return multisampling;
}

VkPipelineRasterizationStateCreateInfo Pipeline::createRasterizer()
{
    VkPipelineRasterizationStateCreateInfo rasterizer{};
    rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterizer.depthClampEnable = VK_FALSE;
    rasterizer.rasterizerDiscardEnable = VK_FALSE;
    rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
    rasterizer.lineWidth = 1.0f;
    rasterizer.cullMode = VK_CULL_MODE_FRONT_BIT; // VK_CULL_MODE_BACK_BIT;
    rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
    rasterizer.depthBiasEnable = VK_FALSE;
    rasterizer.depthBiasConstantFactor = 0.0f; // Optional
    rasterizer.depthBiasClamp = 0.0f;          // Optional
    rasterizer.depthBiasSlopeFactor = 0.0f;    // Optional
    return rasterizer;
}

VkPipelineViewportStateCreateInfo Pipeline::createViewPort()
{
    VkViewport *viewport = new VkViewport();
    viewport->x = 0.0f;
    viewport->y = 0.0f;
    viewport->width = viewPortWidth;
    viewport->height = viewPortWidth;
    viewport->minDepth = 0.0f;
    viewport->maxDepth = 1.0f;

    VkRect2D *scissor = new VkRect2D();
    scissor->offset = {0, 0};
    scissor->extent = extent;

    VkPipelineViewportStateCreateInfo viewportState{};
    viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewportState.viewportCount = 1;
    viewportState.pViewports = viewport;
    viewportState.scissorCount = 1;
    viewportState.pScissors = scissor;
    return viewportState;
}

VkPipelineInputAssemblyStateCreateInfo Pipeline::createInputAssembly()
{
    VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
    inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    inputAssembly.primitiveRestartEnable = VK_FALSE;
    return inputAssembly;
}

VkPipelineVertexInputStateCreateInfo *Pipeline::createVertexInputInfo()
{
    auto bindingDescription = new VkVertexInputBindingDescription;
    *bindingDescription = Vertex::getBindingDescription();
    auto attributeDescriptions = new std::array<VkVertexInputAttributeDescription, 3>;
    *attributeDescriptions = Vertex::getAttributeDescriptions();
    VkPipelineVertexInputStateCreateInfo *vertexInputInfo = new VkPipelineVertexInputStateCreateInfo();
    vertexInputInfo->sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertexInputInfo->vertexBindingDescriptionCount = 1;
    vertexInputInfo->pVertexBindingDescriptions = bindingDescription;
    vertexInputInfo->vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions->size());
    vertexInputInfo->pVertexAttributeDescriptions = attributeDescriptions->data();
    return vertexInputInfo;
}

void Pipeline::createPipelineLayout()
{
    VkDevice device = Graphics::getDevice();
    VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.setLayoutCount = setLayoutCount;   // Optional
    pipelineLayoutInfo.pSetLayouts = descriptorSetLayout; // Optional
    pipelineLayoutInfo.pushConstantRangeCount = 0;        // Optional
    pipelineLayoutInfo.pPushConstantRanges = nullptr;     //&pushConstantRange; // Optional

    if (vkCreatePipelineLayout(device, &pipelineLayoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create pipeline layout!");
    }
}
VkPipelineDepthStencilStateCreateInfo Pipeline::createDepthStencil()
{
    VkPipelineDepthStencilStateCreateInfo depthStencil{};
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

VkShaderModule Pipeline::createShaderModule(const std::vector<char> &code)
{
    VkDevice device = Graphics::getDevice();
    VkShaderModuleCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    createInfo.codeSize = code.size();
    createInfo.pCode = reinterpret_cast<const uint32_t *>(code.data());
    VkShaderModule shaderModule;
    if (vkCreateShaderModule(device, &createInfo, nullptr, &shaderModule) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create shader module!");
    }
    return shaderModule;
}

void Pipeline::loadShaderModule(std::string vertFilePath, std::string fragFilePath)
{
    auto vertShaderCode = getterChecker::readFile(vertFilePath);
    auto fragShaderCode = getterChecker::readFile(fragFilePath);

    VkShaderModule vertShaderModule = createShaderModule(vertShaderCode);
    VkShaderModule fragShaderModule = createShaderModule(fragShaderCode);

    VkPipelineShaderStageCreateInfo vertShaderStageInfo{};
    vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
    vertShaderStageInfo.module = vertShaderModule;
    vertShaderStageInfo.pName = "main";

    VkPipelineShaderStageCreateInfo fragShaderStageInfo{};
    fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    fragShaderStageInfo.module = fragShaderModule;
    fragShaderStageInfo.pName = "main";

    shaderStages[0] = vertShaderStageInfo;
    shaderStages[1] = fragShaderStageInfo;
}

void Pipeline::createDepthOnlyPipeline(uint32_t subpassIndex)
{
    VkDevice device = Graphics::getDevice();
    createPipelineLayout();
    loadShaderModule(vertFilePath, fragFilePath);
    // VkPipelineColorBlendStateCreateInfo colorBlending = createColorBlending();
    VkPipelineMultisampleStateCreateInfo multisampling = createMultiSampling(VK_SAMPLE_COUNT_1_BIT);
    VkPipelineRasterizationStateCreateInfo rasterizer = createRasterizer();
    VkPipelineViewportStateCreateInfo viewportState = createViewPort();
    VkPipelineInputAssemblyStateCreateInfo inputAssembly = createInputAssembly();
    VkPipelineVertexInputStateCreateInfo *vertexInputInfo = createVertexInputInfo();
    VkPipelineDepthStencilStateCreateInfo depthStencil = createDepthStencil();

    VkGraphicsPipelineCreateInfo pipelineInfo{};
    pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipelineInfo.stageCount = 2;
    pipelineInfo.pStages = shaderStages.data();
    pipelineInfo.pVertexInputState = vertexInputInfo;
    pipelineInfo.pInputAssemblyState = &inputAssembly;
    pipelineInfo.pViewportState = &viewportState;
    pipelineInfo.pRasterizationState = &rasterizer;
    pipelineInfo.pMultisampleState = &multisampling;

    pipelineInfo.pDepthStencilState = &depthStencil; // Optional
    pipelineInfo.pDynamicState = nullptr;            // Optional
    pipelineInfo.layout = pipelineLayout;
    pipelineInfo.renderPass = *renderPass;
    pipelineInfo.subpass = subpassIndex;
    // pipelineInfo.pDepthStencilState = &depthStencil;

    if (vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &pipeline) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create graphics pipeline!");
    }
    delete pipelineInfo.pVertexInputState->pVertexAttributeDescriptions;
    delete pipelineInfo.pVertexInputState->pVertexBindingDescriptions;
    delete pipelineInfo.pVertexInputState;
    delete pipelineInfo.pViewportState->pViewports;
    delete pipelineInfo.pViewportState->pScissors;
    vkDestroyShaderModule(device, shaderStages[1].module, nullptr);
    vkDestroyShaderModule(device, shaderStages[0].module, nullptr);
}

void Pipeline::createPipeline(uint32_t subpassIndex)
{
    VkDevice device = Graphics::getDevice();
    createPipelineLayout();
    loadShaderModule(vertFilePath, fragFilePath);
    VkSampleCountFlagBits msaaSamples = Graphics::getMsaaSamples();

    VkPipelineColorBlendStateCreateInfo colorBlending = createColorBlending();
    VkPipelineMultisampleStateCreateInfo multisampling = createMultiSampling(msaaSamples);
    VkPipelineRasterizationStateCreateInfo rasterizer = createRasterizer();
    VkPipelineViewportStateCreateInfo viewportState = createViewPort();
    VkPipelineInputAssemblyStateCreateInfo inputAssembly = createInputAssembly();
    VkPipelineVertexInputStateCreateInfo *vertexInputInfo = createVertexInputInfo();
    VkPipelineDepthStencilStateCreateInfo depthStencil = createDepthStencil();

    VkGraphicsPipelineCreateInfo pipelineInfo{};
    pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipelineInfo.stageCount = 2;
    pipelineInfo.pStages = shaderStages.data();
    pipelineInfo.pVertexInputState = vertexInputInfo;
    pipelineInfo.pInputAssemblyState = &inputAssembly;
    pipelineInfo.pViewportState = &viewportState;
    pipelineInfo.pRasterizationState = &rasterizer;
    pipelineInfo.pMultisampleState = &multisampling;
    pipelineInfo.pDepthStencilState = nullptr; // Optional
    pipelineInfo.pColorBlendState = &colorBlending;
    pipelineInfo.pDynamicState = nullptr; // Optional
    pipelineInfo.layout = pipelineLayout;
    pipelineInfo.renderPass = *renderPass;
    pipelineInfo.subpass = subpassIndex;
    pipelineInfo.pDepthStencilState = &depthStencil;

    if (vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &pipeline) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create graphics pipeline!");
    }
    delete pipelineInfo.pVertexInputState->pVertexAttributeDescriptions;
    delete pipelineInfo.pVertexInputState->pVertexBindingDescriptions;
    delete pipelineInfo.pVertexInputState;
    delete pipelineInfo.pColorBlendState->pAttachments;
    delete pipelineInfo.pViewportState->pViewports;
    delete pipelineInfo.pViewportState->pScissors;
    vkDestroyShaderModule(device, shaderStages[1].module, nullptr);
    vkDestroyShaderModule(device, shaderStages[0].module, nullptr);
}

VkPipeline *Pipeline::getPipeline()
{
    return &pipeline;
}

VkPipelineLayout Pipeline::getPipelineLayout()
{
    return pipelineLayout;
}
