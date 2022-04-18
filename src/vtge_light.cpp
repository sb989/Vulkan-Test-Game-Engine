#include "vtge_light.hpp"
#include "vtge_object.hpp"
#include "vtge_model.hpp"
#include "vtge_mesh.hpp"
#include "vtge_swapchain.hpp"
#include "vtge_camera.hpp"
#include "vtge_pipeline.hpp"
#include "vtge_framebuffer.hpp"
#include "vtge_descriptor.hpp"
#include "vtge_graphics.hpp"
#include "vtge_depth_buffer.hpp"
#include "vtge_renderpass.hpp"
#include "glm/gtx/string_cast.hpp"
#include <iostream>
static std::vector<Light *> directionalLightList, spotLightList, pointLightList;

std::vector<VkBuffer> Light::directionalLightBuffers = {0};
std::vector<VkBuffer> Light::spotLightBuffers = {0};
std::vector<VkBuffer> Light::pointLightBuffers = {0};

std::vector<VkDeviceMemory> Light::directionalLightBuffersMemory = {0};
std::vector<VkDeviceMemory> Light::spotLightBuffersMemory = {0};
std::vector<VkDeviceMemory> Light::pointLightBuffersMemory = {0};

std::vector<VkDescriptorSet> *Light::descriptorSets = nullptr, *Light::shadowMapDescriptorSet;
VkDescriptorSetLayout *Light::descriptorSetLayout = nullptr, *Light::shadowMapDescriptorSetLayout = nullptr;
VkDescriptorPool *Light::descriptorPool = nullptr;
DepthBuffer *Light::directionalShadowDepthBuffer = nullptr;
uint32_t Light::imageCount = 0;

Light::Light(std::string modelPath, glm::vec4 lightPos, uint32_t imageCount,
             glm::vec4 diffuse, glm::vec4 ambient, glm::vec4 specular,
             float constant, float linear, float quadratic, std::string colorName)
{
    // constructor for a point light
    this->m = new Model(modelPath, imageCount, "", "", diffuse * 255.0f, colorName);
    this->lightPos = lightPos;
    this->diffuse = diffuse;
    this->ambient = ambient;
    this->specular = specular;
    this->constant = constant;
    this->linear = linear;
    this->quadratic = quadratic;
    this->type = Point;
    // Descriptor::createDescriptorBuffer(sizeof(UniformBufferObject), &uniformBuffers, &uniformBuffersMemory, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, imageCount);
    if (!descriptorSetLayout)
    {
        this->imageCount = imageCount;
        initLights();
    }
    pointLightList.push_back(this);
    m->moveModel(lightPos);
}

Light::Light(std::string modelPath, glm::vec4 lightPos, glm::vec4 direction, uint32_t imageCount,
             glm::vec4 diffuse, glm::vec4 ambient, glm::vec4 specular, std::string colorName)
{
    // constructor for a directional light
    this->m = new Model(modelPath, imageCount, "", "", diffuse * 255.0f, colorName);
    this->lightPos = lightPos;
    this->direction = direction;
    this->diffuse = diffuse;
    this->ambient = ambient;
    this->specular = specular;
    this->type = Direction;
    this->lightProjection = glm::mat4(1.0f);
    this->lightView = glm::mat4(1.0f);
    if (!descriptorSetLayout)
    {
        this->imageCount = imageCount;
        initLights();
    }
    directionalLightList.push_back(this);
    m->moveModel(lightPos);
}

Light::Light(std::string modelPath, glm::vec4 lightPos, glm::vec4 direction, uint32_t imageCount,
             glm::vec4 diffuse, glm::vec4 ambient, glm::vec4 specular,
             float constant, float linear, float quadratic, float cutOff, float outerCutOff, std::string colorName)
{
    // constructor for a spot light
    this->m = new Model(modelPath, imageCount, "", "", diffuse * 255.0f, colorName);
    this->lightPos = lightPos;
    this->diffuse = diffuse;
    this->ambient = ambient;
    this->specular = specular;
    this->constant = constant;
    this->linear = linear;
    this->quadratic = quadratic;
    this->cutOff = cutOff;
    this->outerCutOff = outerCutOff;
    this->direction = direction;
    this->type = Spot;
    // Descriptor::createDescriptorBuffer(sizeof(UniformBufferObject), &uniformBuffers, &uniformBuffersMemory, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, imageCount);
    if (!descriptorSetLayout)
    {
        this->imageCount = imageCount;
        initLights();
    }
    spotLightList.push_back(this);
    m->moveModel(lightPos);
}

void Light::initLights(int imageCount)
{
    if (imageCount != -1)
        setImageCount(imageCount);

    setupDescriptorSetLayout();
    Descriptor::createDescriptorBuffer(16 + sizeof(DirectionalLightInfo) * MAX_LIGHT_COUNT, &directionalLightBuffers,
                                       &directionalLightBuffersMemory, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, imageCount);
    Descriptor::createDescriptorBuffer(16 + sizeof(SpotLightInfo) * MAX_LIGHT_COUNT, &spotLightBuffers,
                                       &spotLightBuffersMemory, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, imageCount);
    Descriptor::createDescriptorBuffer(16 + sizeof(PointLightInfo) * MAX_LIGHT_COUNT, &pointLightBuffers,
                                       &pointLightBuffersMemory, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, imageCount);
    createDescriptorPool();
    createDescriptorSets();
    directionalShadowDepthBuffer = new DepthBuffer(10, imageCount);

    setupShadowMapDescriptorSetLayouts();
    createShadowMapDescriptorSet();
    populateAllShadowDepthCmdBuffers(imageCount);
}

void Light::populateAllShadowDepthCmdBuffers(uint32_t imageCount)
{
    for (int i = 0; i < imageCount; i++)
    {
        populateShadowDepthCmdBuffers(i);
    }
}

Light::~Light()
{
    delete m;
}

void Light::createDirectionalLight(std::string modelPath, glm::vec3 scale, glm::vec3 rotate, glm::vec3 direction, glm::vec4 lightPos,
                                   glm::vec4 diffuse, glm::vec4 ambient, glm::vec4 specular, std::string colorName)
{
    uint32_t imgCount = Graphics::getSwapchain()->swapchainImages.size();
    Light *l = new Light(modelPath, lightPos, glm::vec4(direction, 0), imgCount, diffuse, ambient, specular, colorName);
    l->calculateLightViewMatrix();
    // glm::vec3 offset = glm::vec3(lightPos.x * direction.x, lightPos.y * direction.y, lightPos.z * direction.z);
    // std::cout << glm::to_string(glm::vec3(lightPos) + offset) << std::endl;
    l->getModel()->rotateModel(rotate);
    l->getModel()->scaleModel(scale);
    l->getModel()->setRotation(rotate / 40.0f);
    // lightList.push_back(m);
}

void Light::createPointLight(std::string modelPath, glm::vec3 scale, glm::vec3 rotate, glm::vec4 lightPos,
                             glm::vec4 diffuse, glm::vec4 ambient, glm::vec4 specular, float constant,
                             float linear, float quadratic, std::string colorName)
{
    uint32_t imgCount = Graphics::getSwapchain()->swapchainImages.size();
    Light *l = new Light(modelPath, lightPos, imgCount, diffuse, ambient,
                         specular, constant, linear, quadratic, colorName);
    l->getModel()->rotateModel(rotate);
    l->getModel()->scaleModel(scale);
    l->getModel()->setRotation(rotate / 40.0f);
    // lightList.push_back(m);
}

void Light::createSpotLight(std::string modelPath, glm::vec3 scale, glm::vec3 rotate, glm::vec3 direction, glm::vec4 lightPos,
                            glm::vec4 diffuse, glm::vec4 ambient, glm::vec4 specular, float constant, float linear, float quadratic,
                            float cutOff, float outerCutOff, std::string colorName)
{
    uint32_t imgCount = Graphics::getSwapchain()->swapchainImages.size();
    Light *l = new Light(modelPath, lightPos, glm::vec4(direction, 0), imgCount, diffuse, ambient,
                         specular, constant, linear, quadratic, cutOff, outerCutOff, colorName);
    l->calculateLightViewMatrix();
    l->getModel()->rotateModel(rotate);
    l->getModel()->scaleModel(scale);
    l->getModel()->setRotation(rotate / 40.0f);
}

void Light::setupDescriptorSetLayout()
{
    auto directionalLayoutBinding = Descriptor::createDescriptorSetLayoutBinding(
        0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, nullptr);
    auto spotLayoutBinding = Descriptor::createDescriptorSetLayoutBinding(
        1, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr);
    auto pointLayoutBinding = Descriptor::createDescriptorSetLayoutBinding(
        2, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr);
    std::vector<VkDescriptorSetLayoutBinding> bindings = {directionalLayoutBinding, spotLayoutBinding, pointLayoutBinding};
    descriptorSetLayout = Descriptor::createDescriptorSetLayout(&bindings);
}

void Light::createDescriptorPool()
{
    std::vector<VkDescriptorPoolSize> poolSizes = {
        {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 3 * imageCount},
        {VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, imageCount * 10},
        {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 3 * imageCount}};
    descriptorPool = Descriptor::createDescriptorPool(100, poolSizes);
}

void Light::createDescriptorSets()
{
    descriptorSets = Descriptor::allocateDescriptorSets(imageCount, *descriptorSetLayout, *descriptorPool);
    std::vector<VtgeBufferInfo> bufferInfos;
    std::vector<VtgeImageInfo> imageInfos;
    VtgeBufferInfo directionalLightBufferInfo{};
    directionalLightBufferInfo.buffer = directionalLightBuffers.data();
    directionalLightBufferInfo.offset = 0;
    directionalLightBufferInfo.range = 16 + sizeof(DirectionalLightInfo) * MAX_LIGHT_COUNT;
    directionalLightBufferInfo.binding = 0;
    directionalLightBufferInfo.type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    directionalLightBufferInfo.descriptorCount = 1;
    VtgeBufferInfo spotLightBufferInfo{};
    spotLightBufferInfo.buffer = spotLightBuffers.data();
    spotLightBufferInfo.offset = 0;
    spotLightBufferInfo.range = 16 + sizeof(SpotLightInfo) * MAX_LIGHT_COUNT;
    spotLightBufferInfo.binding = 1;
    spotLightBufferInfo.type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    spotLightBufferInfo.descriptorCount = 1;
    VtgeBufferInfo pointLightBufferInfo{};
    pointLightBufferInfo.buffer = pointLightBuffers.data();
    pointLightBufferInfo.offset = 0;
    pointLightBufferInfo.range = 16 + sizeof(PointLightInfo) * MAX_LIGHT_COUNT;
    pointLightBufferInfo.binding = 2;
    pointLightBufferInfo.type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    pointLightBufferInfo.descriptorCount = 1;
    bufferInfos.push_back(directionalLightBufferInfo);
    bufferInfos.push_back(pointLightBufferInfo);
    bufferInfos.push_back(spotLightBufferInfo);
    Descriptor::populateDescriptorBuffer(descriptorSets, imageCount, bufferInfos, imageInfos);
}

void Light::calculateLightViewMatrix()
{
    if (type == Spot)
        return;
    float near_plane = -20.0f, far_plane = 30.00f;
    float orthoBound = 14.0f;
    lightProjection = glm::ortho(-orthoBound, orthoBound, -orthoBound, orthoBound, near_plane, far_plane);
    glm::vec3 up = glm::cross(glm::vec3(direction), glm::vec3(0, 1, 0));
    if (up.x == 0 && up.y == 0 && up.z == 0)
        up = glm::cross(glm::vec3(direction), glm::vec3(1, 0, 0));

    up = glm::normalize(up);

    glm::vec3 offset = glm::vec3(lightPos.x * direction.x, lightPos.y * direction.y, lightPos.z * direction.z);
    glm::vec3 start = glm::vec3(lightPos) - offset;
    lightView = glm::lookAt(start, start + glm::vec3(direction), up);
}

void Light::updateAllLights(uint32_t currentImage, glm::mat4 projection, glm::mat4 view)
{
    updateLightBuffers(currentImage, projection, view);
}

void Light::updateLightBuffers(uint32_t currentImage, glm::mat4 projection, glm::mat4 view)
{
    updateDirectionalLightBuffer(currentImage, projection, view, directionalLightList, directionalLightBuffersMemory);
    updateSpotLightBuffer(currentImage, projection, view, spotLightList, spotLightBuffersMemory);
    updatePointLightBuffer(currentImage, projection, view, pointLightList, pointLightBuffersMemory);
}

void Light::updateDirectionalLightBuffer(uint32_t currentImage, glm::mat4 projection, glm::mat4 view,
                                         std::vector<Light *> lightList, std::vector<VkDeviceMemory> lightBuffersMemory)
{
    void *data;
    int lightCount = (int)lightList.size();
    VkDevice device = Graphics::getDevice();
    VkDeviceMemory bufferMemory = lightBuffersMemory[currentImage];
    vkMapMemory(device, bufferMemory, 0, 16 + sizeof(DirectionalLightInfo) * lightList.size(), 0, &data);
    memcpy(data, &(lightCount), sizeof(int));
    char *offset = (char *)data;
    offset = offset + 16;
    DirectionalLightInfo *lightData = (DirectionalLightInfo *)offset;
    for (int i = 0; i < lightCount; i++)
    {
        Light *dLight = lightList[i];
        // DirectionalLightInfo *lightData
        dLight->m->updateModelMat(currentImage, projection, view);
        dLight->lightPos = lightList[i]->m->getModelPos();
        dLight->calculateLightViewMatrix();
        lightData[i].ambient = dLight->ambient;
        lightData[i].diffuse = dLight->diffuse;
        lightData[i].specular = dLight->specular;
        lightData[i].direction = view * dLight->direction;
    }
    vkUnmapMemory(device, bufferMemory);
}
void Light::updatePointLightBuffer(uint32_t currentImage, glm::mat4 projection, glm::mat4 view,
                                   std::vector<Light *> lightList, std::vector<VkDeviceMemory> lightBuffersMemory)
{
    void *data;
    int lightCount = (int)lightList.size();
    VkDevice device = Graphics::getDevice();
    VkDeviceMemory bufferMemory = lightBuffersMemory[currentImage];
    vkMapMemory(device, bufferMemory, 0, 16 + sizeof(PointLightInfo) * lightList.size(), 0, &data);
    memcpy(data, &(lightCount), sizeof(int));
    char *offset = (char *)data;
    offset = offset + 16;
    PointLightInfo *lightData = (PointLightInfo *)offset;
    for (int i = 0; i < lightCount; i++)
    {
        lightList[i]->m->updateModelMat(currentImage, projection, view);
        lightList[i]->lightPos = lightList[i]->m->getModelPos();
        lightData[i].lightpos = view * lightList[i]->lightPos;
        lightData[i].ambient = lightList[i]->ambient;
        lightData[i].diffuse = lightList[i]->diffuse;
        lightData[i].specular = lightList[i]->specular;
        lightData[i].constant = lightList[i]->constant;
        lightData[i].linear = lightList[i]->linear;
        lightData[i].quadratic = lightList[i]->quadratic;
    }
    vkUnmapMemory(device, bufferMemory);
}
void Light::updateSpotLightBuffer(uint32_t currentImage, glm::mat4 projection, glm::mat4 view,
                                  std::vector<Light *> lightList, std::vector<VkDeviceMemory> lightBuffersMemory)
{
    void *data;
    int lightCount = (int)lightList.size();
    VkDevice device = Graphics::getDevice();
    VkDeviceMemory bufferMemory = lightBuffersMemory[currentImage];
    vkMapMemory(device, bufferMemory, 0, 16 + sizeof(SpotLightInfo) * lightList.size(), 0, &data);
    memcpy(data, &(lightCount), sizeof(int));
    char *offset = (char *)data;
    offset = offset + 16;
    SpotLightInfo *lightData = (SpotLightInfo *)offset;
    for (int i = 0; i < lightCount; i++)
    {
        lightList[i]->m->updateModelMat(currentImage, projection, view);
        lightList[i]->lightPos = lightList[i]->m->getModelPos();
        lightData[i].lightpos = view * lightList[i]->lightPos;
        lightData[i].ambient = lightList[i]->ambient;
        lightData[i].diffuse = lightList[i]->diffuse;
        lightData[i].specular = lightList[i]->specular;
        lightData[i].constant = lightList[i]->constant;
        lightData[i].linear = lightList[i]->linear;
        lightData[i].quadratic = lightList[i]->quadratic;
        lightData[i].direction = view * lightList[i]->direction;
        lightData[i].cutOff = lightList[i]->cutOff;
        lightData[i].outerCutOff = lightList[i]->outerCutOff;
    }
    vkUnmapMemory(device, bufferMemory);
}

void Light::drawLight(VkCommandBuffer *commandBuffer, VkPipelineLayout pipelineLayout, int index, int instance)
{
    std::vector<VkDescriptorSet> combinedDescriptorSets = {};
    m->drawModel(combinedDescriptorSets, commandBuffer, pipelineLayout, index);
}

void Light::drawAllLights(VkCommandBuffer *commandBuffer, Pipeline *pipeline, int index)
{
    vkCmdBindPipeline(*commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, *pipeline->getPipeline());
    for (int i = 0; i < directionalLightList.size(); i++)
    {
        directionalLightList[i]->drawLight(commandBuffer, pipeline->getPipelineLayout(), index, i);
    }
    for (int i = 0; i < spotLightList.size(); i++)
    {
        spotLightList[i]->drawLight(commandBuffer, pipeline->getPipelineLayout(), index, i);
    }
    for (int i = 0; i < pointLightList.size(); i++)
    {
        pointLightList[i]->drawLight(commandBuffer, pipeline->getPipelineLayout(), index, i);
    }
}

void Light::recreateAllLights(uint32_t imageCount)
{
    Light::imageCount = imageCount;
    for (int i = 0; i < directionalLightList.size(); i++)
    {
        directionalLightList[i]->m->recreateModel(imageCount);
    }
    for (int i = 0; i < spotLightList.size(); i++)
    {
        spotLightList[i]->m->recreateModel(imageCount);
    }
    for (int i = 0; i < pointLightList.size(); i++)
    {
        pointLightList[i]->m->recreateModel(imageCount);
    }
    recreateUBufferPoolSets();
}

void Light::recreateUBufferPoolSets()
{
    recreateLightBuffer();
    createDescriptorPool();
    createDescriptorSets();
}

void Light::recreateLightBuffer()
{
    Descriptor::createDescriptorBuffer(16 + sizeof(DirectionalLightInfo) * MAX_LIGHT_COUNT, &directionalLightBuffers,
                                       &directionalLightBuffersMemory, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, imageCount);
    Descriptor::createDescriptorBuffer(16 + sizeof(SpotLightInfo) * MAX_LIGHT_COUNT, &spotLightBuffers,
                                       &spotLightBuffersMemory, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, imageCount);
    Descriptor::createDescriptorBuffer(16 + sizeof(PointLightInfo) * MAX_LIGHT_COUNT, &pointLightBuffers,
                                       &pointLightBuffersMemory, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, imageCount);
}

VkDescriptorSetLayout *Light::getDescriptorSetLayout()
{
    return descriptorSetLayout;
}

std::vector<VkDescriptorSet> *Light::getDescriptorSets()
{
    return descriptorSets;
}

glm::vec4 Light::getDirection()
{
    if (type == Point)
        return glm::vec4(1);
    else
        return direction;
}

glm::mat4 Light::getLightView()
{
    if (type == Point)
        return glm::mat4(1);
    else
        return lightView;
}

glm::mat4 Light::getLightProjection()
{
    if (type == Point)
        return glm::mat4(1);
    else
        return lightProjection;
}

glm::vec4 Light::getPos()
{
    return lightPos;
}

Light *Light::getDirectionalLight(uint32_t i)
{
    if (directionalLightList.size() > i)
        return directionalLightList[i];
    else
        return nullptr;
}

Light *Light::getPointLight(uint32_t i)
{
    if (pointLightList.size() > i)
        return pointLightList[i];
    else
        return nullptr;
}

Light *Light::getSpotLight(uint32_t i)
{
    if (spotLightList.size() > i)
        return spotLightList[i];
    else
        return nullptr;
}

std::vector<VkBuffer> *Light::getDirectionalLightBuffers()
{
    return &directionalLightBuffers;
}

std::vector<VkDeviceMemory> *Light::getDirectionalLightBufferMemory()
{
    return &directionalLightBuffersMemory;
}

VkDeviceSize Light::getDirectionalLightBufferSize()
{
    return 16 + sizeof(DirectionalLightInfo) * MAX_LIGHT_COUNT;
}

void Light::destroyDescriptorSetLayout()
{
    VkDevice device = Graphics::getDevice();
    vkDestroyDescriptorSetLayout(device, *descriptorSetLayout, nullptr);
    delete (descriptorSetLayout);
}

void Light::destroyAllLights()
{
    for (int i = 0; i < directionalLightList.size(); i++)
    {
        delete directionalLightList[i];
    }
    for (int i = 0; i < spotLightList.size(); i++)
    {
        delete spotLightList[i];
    }
    for (int i = 0; i < pointLightList.size(); i++)
    {
        delete pointLightList[i];
    }

    directionalLightList.clear();
    spotLightList.clear();
    pointLightList.clear();
}

void Light::destroyLightBufferAndMemory(size_t imageCount, std::vector<VkBuffer> buffer, std::vector<VkDeviceMemory> memory)
{
    VkDevice device = Graphics::getDevice();
    for (size_t j = 0; j < imageCount; j++)
    {
        vkDestroyBuffer(device, buffer[j], nullptr);
        vkFreeMemory(device, memory[j], nullptr);
    }
    buffer.clear();
    memory.clear();
}

void Light::cleanupMemory()
{
    VkDevice device = Graphics::getDevice();
    std::cout << "cleaning up model memory" << std::endl;
    for (int i = 0; i < directionalLightList.size(); i++)
    {
        directionalLightList[i]->m->cleanupMemory();
    }
    for (int i = 0; i < spotLightList.size(); i++)
    {
        spotLightList[i]->m->cleanupMemory();
    }
    for (int i = 0; i < pointLightList.size(); i++)
    {
        pointLightList[i]->m->cleanupMemory();
    }
    vkDestroyDescriptorPool(device, *descriptorPool, nullptr);
    delete descriptorPool;
    delete descriptorSets;
    std::cout << "finised cleaning up model memory" << std::endl;
}

void Light::cleanupAllMemory()
{
    destroyLightBufferAndMemory(imageCount, directionalLightBuffers, directionalLightBuffersMemory);
    destroyLightBufferAndMemory(imageCount, spotLightBuffers, spotLightBuffersMemory);
    destroyLightBufferAndMemory(imageCount, pointLightBuffers, pointLightBuffersMemory);
    cleanupMemory();
}

void Light::setImageCount(uint32_t imageCount)
{
    Light::imageCount = imageCount;
}

/**************************
 * Shadow stuff below
 *  ||  ||  ||  ||  ||  ||
 *  \/  \/  \/  \/  \/  \/
 **************************/
void Light::setupShadowMapDescriptorSetLayouts()
{
    auto shadowLayout = Descriptor::createDescriptorSetLayoutBinding(
        0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr);
    std::vector<VkDescriptorSetLayoutBinding> bindings = {shadowLayout};
    shadowMapDescriptorSetLayout = Descriptor::createDescriptorSetLayout(&bindings);
}

void Light::createShadowMapDescriptorSet()
{
    // creates a descriptor set for the second render pass. it holds a sampler to the depth buffer created in the first render pass
    // depth buffer used in the second render pass to draw shadows
    shadowMapDescriptorSet = Descriptor::allocateDescriptorSets(imageCount, *shadowMapDescriptorSetLayout, *descriptorPool);
    std::cout << "create shadow map descriptor set" << std::endl;
    std::vector<VtgeBufferInfo> bufferInfos;
    std::vector<VtgeImageInfo> imageInfos;
    Framebuffer *fb = directionalShadowDepthBuffer->getFramebuffer();
    VtgeImageInfo depthSamplerInfo{};
    depthSamplerInfo.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;
    depthSamplerInfo.view = fb->getCombinedShadowImageView();
    depthSamplerInfo.sampler = *directionalShadowDepthBuffer->getSampler();
    depthSamplerInfo.binding = 0;
    depthSamplerInfo.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    depthSamplerInfo.descriptorCount = 1;
    imageInfos.push_back(depthSamplerInfo);
    Descriptor::populateDescriptorBuffer(shadowMapDescriptorSet, imageCount, bufferInfos, imageInfos);
}

void Light::populateAndDrawShadows(uint32_t index)
{
    VkDevice device = Graphics::getDevice();
    std::vector<VkCommandBuffer> cmdBuffers = directionalShadowDepthBuffer->getDepthCommandBuffers();
    VkCommandBuffer cmdBuffer = cmdBuffers[index];
    vkResetCommandBuffer(cmdBuffer, 0);
    drawShadows(index);
    std::vector<VkSemaphore> signalSemaphores = {directionalShadowDepthBuffer->getDepthSemaphore()[0]};
    std::vector<VkSemaphore> waitSemaphores = {}; // do i need to wait for anything?
    std::vector<VkPipelineStageFlags> waitStages = {};

    Graphics::submitQueue(
        &cmdBuffer,
        VK_NULL_HANDLE, &signalSemaphores, &waitSemaphores, &waitStages);
}

void Light::populateShadowDepthCmdBuffers(uint32_t index)
{
    //    VkCommandBuffer cmdBuffer = (directionalShadowDepthBuffer->getDepthCommandBuffers())[index];
    drawShadows(index);
}

void Light::drawShadows(uint32_t index)
{
    std::vector<VkCommandBuffer> cmdBuffers = directionalShadowDepthBuffer->getDepthCommandBuffers();
    VkCommandBuffer cmdBuffer = cmdBuffers[index];
    VkRenderPass *renderP = directionalShadowDepthBuffer->getRenderPass();
    // VkCommandBuffer cmdBuffer = Graphics::getDrawCommandBuffer()[index];
    Pipeline *pipe = directionalShadowDepthBuffer->getPipeline();
    Framebuffer *depthFramebuffer = directionalShadowDepthBuffer->getFramebuffer();
    Graphics::beginCommandBuffer(cmdBuffer, "shadow");
    for (int i = 0; i < directionalLightList.size(); i++)
    {
        RenderPass::startShadowRenderPass(renderP, depthFramebuffer->framebuffers[i], cmdBuffer);
        vkCmdBindPipeline(cmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, *pipe->getPipeline());
        directionalLightList[i]->drawShadow(index, i);
        vkCmdEndRenderPass(cmdBuffer);
    }
    Graphics::endCommandBuffer(cmdBuffer);
}

void Light::drawShadow(uint32_t index, uint32_t lightNum)
{
    std::vector<Object *> objList = Object::getObjectList();
    int i = 0;
    for (Object *obj : objList)
    {

        if (i == 0)
        {
            Model *m = obj->getModel();
            glm::mat4 modelMat = m->getModelMat();
            Camera *cam = Graphics::getCamera();

            glm::mat4 modelViewLightMat = cam->getViewMat() * modelMat;

            // std::cout << glm::to_string(modelViewLightMat) << "used in shadow object 0" << std::endl;
        }
        directionalShadowDepthBuffer->drawDepthBuffer(obj, index, lightNum);
        i++;
    }
}

std::vector<VkDescriptorSet> *Light::getShadowMapDescriptorSet()
{
    return shadowMapDescriptorSet;
}

VkDescriptorSetLayout *Light::getShadowMapDescriptorSetLayout()
{
    return shadowMapDescriptorSetLayout;
}

DepthBuffer *Light::getDepthBuffer()
{
    return directionalShadowDepthBuffer;
}