#include "vtge_depth_buffer.hpp"
#include "vtge_graphics.hpp"
#include "vtge_descriptor.hpp"
#include "vtge_light.hpp"
#include "vtge_framebuffer.hpp"
#include "vtge_object.hpp"
#include "vtge_renderpass.hpp"
#include "vtge_pipeline.hpp"
#include "vtge_mesh.hpp"
#include "vtge_model.hpp"
#include "vtge_camera.hpp"
#include "glm/gtx/string_cast.hpp"
#include <iostream>

DepthBuffer::DepthBuffer(uint32_t layers, uint32_t imageCount)
{
    this->layersCount = layers;
    this->imageCount = imageCount;
    VkSampleCountFlagBits msaa = Graphics::getMsaaSamples();
    RenderPass::createShadowMapRenderPass(&renderPass, msaa);
    this->framebuffer = new Framebuffer(&renderPass, layers);
    setupDescriptorPool();
    // setupDepthDescriptorSetLayouts();
    VkDescriptorSetLayout depthPipelineLayouts[1] = {*Mesh::getShadowDescriptorSetLayout()};
    depthPipeline = Pipeline::createShadowPipeline("../shaders/shadow_vert.spv", "../shaders/shadow_frag.spv", &renderPass, depthPipelineLayouts, 1, 0);
    // createDepthDescriptorBuffers();
    //  for (int i = 0; i < layers; i++)
    //  {
    //      createDepthDescriptorSets();
    //  }
    Graphics::allocateCommandBuffer(&depthCommandBuffers, 3);
    createDepthSampler();
    setupSemaphore();
    VkDevice device = Graphics::getDevice();
}

void DepthBuffer::setupDescriptorPool()
{
    std::vector<VkDescriptorPoolSize> poolSizes = {
        {VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, imageCount * MAX_LIGHT_COUNT},
        {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 50}};
    descriptorPool = Descriptor::createDescriptorPool(1000, poolSizes);
}

void DepthBuffer::setupDepthDescriptorSetLayouts()
{
    // auto modelLightLayout = Descriptor::createDescriptorSetLayoutBinding(
    //     0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_VERTEX_BIT, nullptr);
    // auto meshuboLayout = Descriptor::createDescriptorSetLayoutBinding(
    //     1, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_VERTEX_BIT, nullptr);
    // std::vector<VkDescriptorSetLayoutBinding> bindings = {modelLightLayout, meshuboLayout};
    auto uboLayoutBinding = Descriptor::createDescriptorSetLayoutBinding(
        0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_VERTEX_BIT, nullptr);
    auto samplerLayoutBinding = Descriptor::createDescriptorSetLayoutBinding(
        1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr);
    auto specMapLayoutBinding = Descriptor::createDescriptorSetLayoutBinding(
        2, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr);
    auto materialLayoutBinding = Descriptor::createDescriptorSetLayoutBinding(
        3, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr);
    std::vector<VkDescriptorSetLayoutBinding> bindings = {uboLayoutBinding, samplerLayoutBinding, materialLayoutBinding, specMapLayoutBinding};

    depthDescriptorSetLayout = Descriptor::createDescriptorSetLayout(&bindings);
}

void DepthBuffer::createDepthDescriptorSets()
{
    // create descriptor set for first render pass that write to the depth buffer
    std::vector<VkDescriptorSet> *depthSet = Descriptor::allocateDescriptorSets(1, *depthDescriptorSetLayout, *descriptorPool);
    std::vector<VtgeBufferInfo> bufferInfos;
    std::vector<VtgeImageInfo> imageInfos;
    VtgeBufferInfo modelLightBufferInfo{};
    modelLightBufferInfo.binding = 0;
    modelLightBufferInfo.buffer = shadowModelLightMatBuffers.data();
    modelLightBufferInfo.offset = 0;
    modelLightBufferInfo.range = sizeof(ModelLightMatrix);
    modelLightBufferInfo.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    modelLightBufferInfo.descriptorCount = 1;

    VtgeBufferInfo meshUboInfo{};
    meshUboInfo.binding = 1;
    meshUboInfo.buffer = uboMeshBuffers.data();
    meshUboInfo.offset = 0;
    meshUboInfo.range = sizeof(UniformBufferObject);
    meshUboInfo.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    meshUboInfo.descriptorCount = 1;
    bufferInfos.push_back(modelLightBufferInfo);
    bufferInfos.push_back(meshUboInfo);
    Descriptor::populateDescriptorBuffer(depthSet, 1, bufferInfos, imageInfos);
    depthDescriptorSets.push_back(*depthSet);
}

void DepthBuffer::createDepthSampler()
{
    VkDevice device = Graphics::getDevice();
    VkSamplerCreateInfo sampler{};
    sampler.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    sampler.magFilter = VK_FILTER_LINEAR;
    sampler.minFilter = VK_FILTER_LINEAR;
    sampler.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
    sampler.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    sampler.addressModeV = sampler.addressModeU;
    sampler.addressModeW = sampler.addressModeU;
    sampler.mipLodBias = 0.0f;
    sampler.maxAnisotropy = 1.0f;
    sampler.minLod = 0.0f;
    sampler.maxLod = 0.0f;
    sampler.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;
    vkCreateSampler(device, &sampler, nullptr, &depthSampler);
}

void DepthBuffer::createDepthDescriptorBuffers()
{
    Descriptor::createDescriptorBuffer(sizeof(ModelLightMatrix), &shadowModelLightMatBuffers, &shadowModelLightMatBuffersMemory, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, imageCount);
    Descriptor::createDescriptorBuffer(sizeof(UniformBufferObject), &uboMeshBuffers, &uboMeshMemory, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, imageCount);
}

// void DepthBuffer::updateModelLightMat(uint32_t currentImage, ModelLightMatrix mlm)
// {
//     void *data;
//     VkDevice device = Graphics::getDevice();
//     vkMapMemory(device, shadowModelLightMatBuffersMemory[currentImage], 0, sizeof(mlm), 0, &data);
//     memcpy(data, &mlm, sizeof(mlm));
//     vkUnmapMemory(device, shadowModelLightMatBuffersMemory[currentImage]);
// }

void DepthBuffer::drawDepthBuffer(Object *obj, uint32_t index, uint32_t lightNum)
{
    Light *dLight = Light::getDirectionalLight(lightNum);
    Model *model = obj->getModel();
    std::vector<Mesh *> meshes = model->getMeshList();
    glm::mat4 modelMat = model->getModelMat();
    glm::mat4 lightView = dLight->getLightView();
    glm::mat4 lightProjection = dLight->getLightProjection();
    Camera *cam = Graphics::getCamera();

    // ModelLightMatrix mlm;
    // mlm.projLightMat = lightProjection; // cam->getProjectionMat();
    // glm::mat4 modelViewLightMat = lightView * modelMat;

    for (Mesh *mesh : meshes)
    {
        mesh->updateShadowBuffers(modelMat, lightView, lightProjection, index);
        std::vector<VkDescriptorSet> combinedDescriptorSets = {(*mesh->getShadowDescriptorSets())[index]};
        VkBuffer vertexBuffers[] = {mesh->getVertexBuffer()};
        VkBuffer indexBuffer = mesh->getIndexBuffer();
        std::vector<uint32_t> indices = mesh->getIndices();
        VkDeviceSize offsets[] = {0};
        VkCommandBuffer commandBuffer = depthCommandBuffers[index];
        vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);
        vkCmdBindIndexBuffer(commandBuffer, indexBuffer, 0, VK_INDEX_TYPE_UINT32);
        vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
                                depthPipeline->getPipelineLayout(), 0, combinedDescriptorSets.size(), combinedDescriptorSets.data(), 0, nullptr);
        vkCmdDrawIndexed(commandBuffer, static_cast<uint32_t>(indices.size()), 1, 0, 0, 0);
    }
}

std::vector<VkCommandBuffer> DepthBuffer::getDepthCommandBuffers()
{
    return depthCommandBuffers;
}

std::vector<VkSemaphore> DepthBuffer::getDepthSemaphore()
{
    return renderFinishedSemaphore;
}

void DepthBuffer::setupSemaphore()
{
    VkDevice device = Graphics::getDevice();
    size_t maxFramesInFlight = 1; // Graphics::getMaxFramesInFlight();
    renderFinishedSemaphore.resize(maxFramesInFlight);
    VkSemaphoreCreateInfo semaphoreInfo{};
    semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
    // for (size_t i = 0; i < maxFramesInFlight; i++)
    //{
    if (vkCreateSemaphore(device, &semaphoreInfo, nullptr, &renderFinishedSemaphore[0]) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create sync objects for a frame!");
    }
    //}
}

VkRenderPass *DepthBuffer::getRenderPass()
{
    return &renderPass;
}

VkSampler *DepthBuffer::getSampler()
{
    return &depthSampler;
}

Pipeline *DepthBuffer::getPipeline()
{
    return depthPipeline;
}

Framebuffer *DepthBuffer::getFramebuffer()
{
    return framebuffer;
}