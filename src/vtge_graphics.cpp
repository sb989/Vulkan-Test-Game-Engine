#define GLFW_INCLUDE_VULKAN
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define STB_IMAGE_IMPLEMENTATION
#define GLM_ENABLE_EXPERIMENTAL
#include "vtge_graphics.hpp"
#include "vtge_debug_helper_functions.hpp"
#include "vtge_swapchain.hpp"
#include "vtge_framebuffer.hpp"
#include "vtge_pipeline.hpp"
#include "vtge_object.hpp"
#include "vtge_model.hpp"
#include "vtge_camera.hpp"
#include "vtge_mesh.hpp"
#include "vtge_light.hpp"
#include "vtge_texture.hpp"
#include "vtge_getter_and_checker_functions.hpp"
#include "vtge_renderpass.hpp"
#include "vtge_depth_buffer.hpp"
#include <glm/glm.hpp>
#include <glm/gtx/hash.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "glm/gtx/string_cast.hpp"
#include <glm/gtx/norm.hpp>
#include <type_traits>
#include <chrono>
#include <array>
#include <set>
#include <iostream>
#include <memory>
#include <cstdlib>

VkSampleCountFlagBits msaaSamples = VK_SAMPLE_COUNT_1_BIT;
VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
Swapchain *Graphics::swapchain = nullptr;
SwapchainSupportDetails *Graphics::swapchainSupport = {};
Framebuffer *Graphics::framebuffer = nullptr;
VkInstance Graphics::instance = {};
VkRenderPass Graphics::renderPass = {};
Pipeline *Graphics::graphicsPipeline = nullptr,
         *Graphics::lightPipeline = nullptr;
VkQueue Graphics::graphicsQueue = {},
        Graphics::presentQueue = {},
        Graphics::transferQueue = {};
VkCommandPool Graphics::graphicsCommandPool = {},
              Graphics::transferCommandPool = {};
QueueFamilyIndices Graphics::indices = {};
VkCommandBuffer Graphics::transferCommandBuffer = {},
                Graphics::graphicsCommandBuffer;
std::vector<VkCommandBuffer> Graphics::drawCommandBuffers = {};
VkSurfaceKHR Graphics::surface = {};
VkDevice Graphics::device = {};
Camera *Graphics::cam = nullptr;
extern bool enableValidationLayers;

Graphics::Graphics(uint32_t width, uint32_t height, std::string windowTitle)
{
    WIDTH = width;
    HEIGHT = height;
    swapchainSupport = new SwapchainSupportDetails();
    this->windowTitle = windowTitle;
    setUpWindow();
    setUpGraphics();
    this->cam = new Camera(swapchain->swapchainExtent.width, swapchain->swapchainExtent.height);
}

Graphics::~Graphics()
{
    delete swapchainSupport;
    std::cout << "cleanup swapchain" << std::endl;
    cleanupSwapchain();
    Light::destroyDescriptorSetLayout();
    Mesh::destroyDescriptorSetLayout();
    std::cout << "delete model list" << std::endl;
    Object::destroyAllObjects();
    Light::destroyAllLights();
    Texture::destroyAllTextures();
    std::cout << "done" << std::endl;
    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
    {
        vkDestroySemaphore(device, renderFinishedSemaphores[i], nullptr);
        vkDestroySemaphore(device, imageAvailableSemaphores[i], nullptr);
        vkDestroyFence(device, inFlightFences[i], nullptr);
    }
    vkDestroyCommandPool(device, graphicsCommandPool, nullptr);
    vkDestroyCommandPool(device, transferCommandPool, nullptr);
    vkDestroyDevice(device, nullptr);
    if (enableValidationLayers)
    {
        debug::DestroyDebugUtilsMessengerEXT(instance, &debugMessenger, nullptr);
    }
    vkDestroySurfaceKHR(instance, surface, nullptr);
    vkDestroyInstance(instance, nullptr);
    glfwDestroyWindow(window);
    glfwTerminate();
}

void Graphics::setUpWindow()
{
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    window = glfwCreateWindow(WIDTH, HEIGHT, windowTitle.c_str(), nullptr, nullptr);
    glfwSetWindowUserPointer(window, this);
    glfwSetFramebufferSizeCallback(window, framebufferResizeCallback);
}

void Graphics::setUpGraphics()
{
    createInstance();
    debug::setupDebugMessenger(instance, &debugMessenger);
    createSurface();
    pickPhysicalDevice();
    createLogicalDevice();
    swapchain = new Swapchain(&surface, window, swapchainSupport);
    RenderPass::createDisplayRenderPass(&renderPass, msaaSamples, swapchain);
    createCommandPool();
    beginGraphicsCommandBuffer();
    beginTransferCommandBuffer();
    std::cout << "creating framebuffer now!" << std::endl;
    framebuffer = new Framebuffer(swapchain, &renderPass);
    std::cout << "finished creating framebuffer creating model now!" << std::endl;
    std::cout << "creating objects" << std::endl;
    Mesh::initMeshSystem();
    Light::initLights(swapchain->swapchainImages.size());
    VkDescriptorSetLayout lightPipelineLayouts[1] = {*Mesh::getDescriptorSetLayout()};
    VkDescriptorSetLayout graphicPipelineLayouts[3] = {*Mesh::getDescriptorSetLayout(), *Light::getDescriptorSetLayout(), *Light::getShadowMapDescriptorSetLayout()};
    graphicsPipeline = Pipeline::createDrawingPipeline("../shaders/obj_vert.spv", "../shaders/obj_frag.spv", swapchain, &renderPass, graphicPipelineLayouts, 3, 0);
    lightPipeline = Pipeline::createDrawingPipeline("../shaders/light_vert.spv", "../shaders/light_frag.spv", swapchain, &renderPass, lightPipelineLayouts, 1, 0);
    endTransferCommandBuffer();
    endGraphicsCommandBuffer();
    buffer::cleanupStagingBuffers();
    allocateCommandBuffers();
    createSyncObjects();
}

void Graphics::createInstance()
{
    if (enableValidationLayers && !getterChecker::checkValidationLayerSupport(validationLayers))
    {
        throw std::runtime_error("validation layers requested, but not available!");
    }
    VkApplicationInfo appInfo{};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = "test Engine";
    appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.pEngineName = "No Engine";
    appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.apiVersion = VK_API_VERSION_1_0;
    VkInstanceCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.pApplicationInfo = &appInfo;
    uint32_t glfwExtensionCount = 0;
    const char **glfwExtensions;
    glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
    auto extensions = getterChecker::getRequiredExtensions();
    createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
    createInfo.ppEnabledExtensionNames = extensions.data();
    VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo{};
    if (enableValidationLayers)
    {
        createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
        createInfo.ppEnabledLayerNames = validationLayers.data();

        debug::populateDebugMessengerCreateInfo(debugCreateInfo);
        createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT *)&debugCreateInfo;
    }
    else
    {
        createInfo.enabledLayerCount = 0;
        createInfo.pNext = nullptr;
    }
    if (vkCreateInstance(&createInfo, nullptr, &instance) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create instance!");
    }
    uint32_t extensionCount = 0;
    vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);
    std::vector<VkExtensionProperties> exts(extensionCount);
    vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, exts.data());
    std::cout << "available extensions:\n";
    for (const auto &extension : exts)
    {
        std::cout << '\t' << extension.extensionName << '\n';
    }
}

void Graphics::createSurface()
{
    if (glfwCreateWindowSurface(instance, window, nullptr, &surface) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create window surface!");
    }
}

void Graphics::pickPhysicalDevice()
{
    uint32_t deviceCount = 0;
    vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);

    if (deviceCount == 0)
    {
        throw std::runtime_error("failed to find GPUs with Vulkan support!");
    }
    std::cout << "device count is " << deviceCount << std::endl;
    std::vector<VkPhysicalDevice> devices(deviceCount);
    vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data());
    for (const auto &device : devices)
    {
        querySwapchainSupport(device, swapchainSupport);
        indices = findQueueFamilies(device);
        if (getterChecker::isDeviceSuitable(device, *swapchainSupport, deviceExtensions))
        {
            physicalDevice = device;
            msaaSamples = getterChecker::getMaxUsableSampleCount();
            break;
        }
    }

    if (physicalDevice == VK_NULL_HANDLE)
    {
        throw std::runtime_error("failed to find a suitable GPU!");
    }
}

void Graphics::createLogicalDevice()
{
    std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
    std::set<uint32_t> uniqueQueueFamilies = {indices.graphicsFamily.value(),
                                              indices.presentFamily.value(), indices.transferFamily.value()};
    float queuePriority = 1.0f;

    for (uint32_t queueFamily : uniqueQueueFamilies)
    {
        VkDeviceQueueCreateInfo queueCreateInfo{};
        queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queueCreateInfo.queueFamilyIndex = queueFamily;
        queueCreateInfo.queueCount = 1;
        queueCreateInfo.pQueuePriorities = &queuePriority;
        queueCreateInfos.push_back(queueCreateInfo);
    }

    VkPhysicalDeviceFeatures deviceFeatures{};
    deviceFeatures.samplerAnisotropy = VK_TRUE;
    VkDeviceCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    createInfo.pQueueCreateInfos = queueCreateInfos.data();
    createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
    createInfo.pEnabledFeatures = &deviceFeatures;
    createInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size());
    createInfo.ppEnabledExtensionNames = deviceExtensions.data();
    if (enableValidationLayers)
    {
        createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
        createInfo.ppEnabledLayerNames = validationLayers.data();
    }
    else
    {
        createInfo.enabledLayerCount = 0;
    }

    if (vkCreateDevice(physicalDevice, &createInfo, nullptr, &device) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create logical device!");
    }

    vkGetDeviceQueue(device, indices.graphicsFamily.value(), 0, &graphicsQueue);
    vkGetDeviceQueue(device, indices.presentFamily.value(), 0, &presentQueue);
    vkGetDeviceQueue(device, indices.transferFamily.value(), 0, &transferQueue);
}

void Graphics::createCommandPool()
{
    VkCommandPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolInfo.queueFamilyIndex = indices.graphicsFamily.value();
    poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT; // Optional
    if (vkCreateCommandPool(device, &poolInfo, nullptr, &graphicsCommandPool) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create command pool!");
    }
    poolInfo.queueFamilyIndex = indices.transferFamily.value();
    // this flag should be used for pools that have shortlived buffers
    poolInfo.flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT;
    if (vkCreateCommandPool(device, &poolInfo, nullptr, &transferCommandPool) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create command pool for transfer queue!");
    }
}

void Graphics::allocateCommandBuffers()
{
    allocateCommandBuffer(&drawCommandBuffers, framebuffer->framebuffers.size());
    for (size_t i = 0; i < drawCommandBuffers.size(); i++)
    {
        populateDrawCommandBuffer(i);
    }
}

void Graphics::allocateCommandBuffer(std::vector<VkCommandBuffer> *commandBuffer, uint32_t commandBufferCount)
{
    commandBuffer->resize(commandBufferCount);
    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool = graphicsCommandPool;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount = (uint32_t)commandBuffer->size();

    if (vkAllocateCommandBuffers(device, &allocInfo, commandBuffer->data()) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to allocate command buffers!");
    }
}

void Graphics::populateDrawCommandBuffer(size_t index)
{
    VkCommandBuffer cmdBuffer = drawCommandBuffers[index];
    beginCommandBuffer(cmdBuffer, "draw");
    RenderPass::startDisplayRenderPass(&renderPass, framebuffer->framebuffers[index], swapchain->swapchainExtent, cmdBuffer);
    Object::drawAllObjects(&cmdBuffer, graphicsPipeline, index);
    Light::drawAllLights(&cmdBuffer, lightPipeline, index);
    vkCmdEndRenderPass(cmdBuffer);
    endCommandBuffer(cmdBuffer);
}

void Graphics::beginCommandBuffer(VkCommandBuffer cmdBuffer, std::string cmdBufferName)
{
    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = 0;
    beginInfo.pInheritanceInfo = nullptr;
    if (vkBeginCommandBuffer(cmdBuffer, &beginInfo) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to begin " + cmdBufferName + " command buffer");
    }
}

void Graphics::endCommandBuffer(VkCommandBuffer cmdBuffer)
{
    // vkCmdEndRenderPass(cmdBuffer);
    if (vkEndCommandBuffer(cmdBuffer) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to record command buffer!");
    }
}
void Graphics::createSyncObjects()
{
    imageAvailableSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
    renderFinishedSemaphores.resize(MAX_FRAMES_IN_FLIGHT);

    inFlightFences.resize(MAX_FRAMES_IN_FLIGHT);
    imagesInFlight.resize(swapchain->swapchainImages.size(), VK_NULL_HANDLE);
    VkSemaphoreCreateInfo semaphoreInfo{};
    semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
    VkFenceCreateInfo fenceInfo{};
    fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;
    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
    {
        if (vkCreateSemaphore(device, &semaphoreInfo, nullptr, &imageAvailableSemaphores[i]) != VK_SUCCESS ||
            vkCreateSemaphore(device, &semaphoreInfo, nullptr, &renderFinishedSemaphores[i]) != VK_SUCCESS ||
            vkCreateFence(device, &fenceInfo, nullptr, &inFlightFences[i]) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to create sync objects for a frame!");
        }
    }
}

bool Graphics::waitForFence(uint32_t &imageIndex)
{
    vkWaitForFences(device, 1, &inFlightFences[currentFrame], VK_TRUE, UINT64_MAX);

    VkResult result = vkAcquireNextImageKHR(device, swapchain->swapchain, UINT64_MAX,
                                            imageAvailableSemaphores[currentFrame], VK_NULL_HANDLE, &imageIndex);

    if (result == VK_ERROR_OUT_OF_DATE_KHR)
    {
        recreateSwapchain();
        return false;
    }
    else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
    {
        throw std::runtime_error("failed to acquire swap chain image!");
    }

    if (imagesInFlight[imageIndex] != VK_NULL_HANDLE)
    {
        vkWaitForFences(device, 1, &imagesInFlight[imageIndex], VK_TRUE, UINT64_MAX);
    }
    imagesInFlight[imageIndex] = inFlightFences[currentFrame];
    return true;
}

void Graphics::submitQueue(VkCommandBuffer *commandBuffer, VkFence *fence, std::vector<VkSemaphore> *signalSemaphores,
                           std::vector<VkSemaphore> *waitSemaphores, std::vector<VkPipelineStageFlags> *waitStages)
{

    VkSubmitInfo submitInfo{};
    VkFence fenceVal;
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.waitSemaphoreCount = waitSemaphores->size();
    submitInfo.pWaitSemaphores = waitSemaphores->data();
    submitInfo.pWaitDstStageMask = waitStages->data();
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = commandBuffer;
    submitInfo.signalSemaphoreCount = signalSemaphores->size();
    submitInfo.pSignalSemaphores = signalSemaphores->data();
    if (fence != VK_NULL_HANDLE)
    {
        vkResetFences(device, 1, fence);
        fenceVal = *fence;
    }
    if (fence == VK_NULL_HANDLE)
        fenceVal = VK_NULL_HANDLE;
    if (vkQueueSubmit(graphicsQueue, 1, &submitInfo, fenceVal) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to submit draw command buffer!");
    }
}

void Graphics::presentQueueToScreen(uint32_t &imageIndex, std::vector<VkSemaphore> *signalSemaphores)
{
    VkPresentInfoKHR presentInfo{};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = signalSemaphores->data();
    VkSwapchainKHR swapchains[] = {swapchain->swapchain};
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = swapchains;
    presentInfo.pImageIndices = &imageIndex;
    presentInfo.pResults = nullptr; // Optional
    VkResult result = vkQueuePresentKHR(presentQueue, &presentInfo);
    if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || framebufferResized)
    {
        framebufferResized = false;
        recreateSwapchain();
    }
    else if (result != VK_SUCCESS)
    {
        throw std::runtime_error("failed to present swap chain image!");
    }
}

void Graphics::drawFrame()
{
    uint32_t imageIndex;
    waitForFence(imageIndex);
    // vkResetCommandBuffer(drawCommandBuffers[imageIndex], 0);

    float x = cos(glfwGetTime() / 4.0f) / 4.0f;
    float z = sin(glfwGetTime() / 4.0f) / 4.0f;
    glm::vec3 displacement(x, 0.0f, z);
    Light *l = Light::getPointLight(0);
    if (l)
    {
        Model *m = l->getModel();
        m->moveModel(displacement);
    }
    cam->handleKeyPress(window);
    cam->handleMouse(window);
    cam->updateCamera();
    glm::mat4 projectionMat = cam->getProjectionMat();
    glm::mat4 viewMat = cam->getViewMat();
    Object::updateAllObjects(imageIndex, projectionMat, viewMat);
    Light::updateAllLights(imageIndex, projectionMat, viewMat);
    Light::populateAndDrawShadows(imageIndex);
    populateAndDrawObjects(imageIndex);
    currentFrame = (currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
}

void Graphics::populateAndDrawObjects(uint32_t imageIndex)
{
    std::vector<VkSemaphore> signalSemaphores = {renderFinishedSemaphores[currentFrame]};
    std::vector<VkSemaphore> waitSemaphores = {imageAvailableSemaphores[currentFrame], Light::getDepthBuffer()->getDepthSemaphore()[0]};
    std::vector<VkPipelineStageFlags> waitStages = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};

    vkResetCommandBuffer(drawCommandBuffers[imageIndex], 0);
    populateDrawCommandBuffer(imageIndex);

    submitQueue(&drawCommandBuffers[imageIndex], &inFlightFences[currentFrame], &signalSemaphores, &waitSemaphores, &waitStages);
    presentQueueToScreen(imageIndex, &signalSemaphores);
}

QueueFamilyIndices Graphics::findQueueFamilies(VkPhysicalDevice device)
{
    QueueFamilyIndices ind;
    // Assign index to queue families that could be found
    uint32_t queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);
    std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

    int i = 0;
    for (const auto &queueFamily : queueFamilies)
    {
        VkBool32 presentSupport = false;
        vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &presentSupport);
        if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT)
        {
            ind.graphicsFamily = i;
        }
        else if (queueFamily.queueFlags & VK_QUEUE_TRANSFER_BIT)
        {
            ind.transferFamily = i;
        }
        if (presentSupport)
        {
            ind.presentFamily = i;
        }
        if (ind.isComplete())
        {
            break;
        }
        i++;
    }
    if (!ind.transferFamily.has_value())
    {
        ind.transferFamily = ind.graphicsFamily;
    }
    return ind;
}

void Graphics::framebufferResizeCallback(GLFWwindow *window, int width, int height)
{
    auto app = reinterpret_cast<Graphics *>(glfwGetWindowUserPointer(window));
    app->framebufferResized = true;
}

VkCommandBuffer Graphics::beginSingleTimeCommands(VkCommandPool pool)
{
    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandPool = pool;
    allocInfo.commandBufferCount = 1;
    VkCommandBuffer commandBuffer;
    vkAllocateCommandBuffers(device, &allocInfo, &commandBuffer);
    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    vkBeginCommandBuffer(commandBuffer, &beginInfo);
    return commandBuffer;
}

void Graphics::beginGraphicsCommandBuffer()
{
    graphicsCommandBuffer = beginSingleTimeCommands(graphicsCommandPool);
}

void Graphics::beginTransferCommandBuffer()
{
    transferCommandBuffer = beginSingleTimeCommands(transferCommandPool);
}

void Graphics::endSingleTimeCommands(VkCommandBuffer commandBuffer, VkCommandPool pool, VkQueue queue)
{
    vkEndCommandBuffer(commandBuffer);
    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffer;
    vkQueueSubmit(queue, 1, &submitInfo, VK_NULL_HANDLE);
    vkQueueWaitIdle(queue);
    vkFreeCommandBuffers(device, pool, 1, &commandBuffer);
}

void Graphics::endGraphicsCommandBuffer()
{
    endSingleTimeCommands(graphicsCommandBuffer, graphicsCommandPool, graphicsQueue);
}

void Graphics::endTransferCommandBuffer()
{
    endSingleTimeCommands(transferCommandBuffer, transferCommandPool, transferQueue);
}

void Graphics::cleanupSwapchain()
{
    delete framebuffer;
    vkFreeCommandBuffers(device, graphicsCommandPool, static_cast<uint32_t>(drawCommandBuffers.size()), drawCommandBuffers.data());
    delete graphicsPipeline;
    delete lightPipeline;
    vkDestroyRenderPass(device, renderPass, nullptr);
    Object::cleanupAllMemory();
    Light::cleanupAllMemory();
    delete swapchain;
}

void Graphics::recreateSwapchain()
{
    std::cout << "recreating swapchain" << std::endl;
    int width = 0, height = 0;
    glfwGetFramebufferSize(window, &width, &height);
    while (width == 0 || height == 0)
    {
        glfwGetFramebufferSize(window, &width, &height);
        glfwWaitEvents();
    }
    vkDeviceWaitIdle(device);
    cleanupSwapchain();
    querySwapchainSupport(physicalDevice, swapchainSupport);

    swapchain = new Swapchain(&surface, window, swapchainSupport);

    RenderPass::createDisplayRenderPass(&renderPass, msaaSamples, swapchain);
    VkDescriptorSetLayout graphicPipelineLayouts[3] = {*Mesh::getDescriptorSetLayout(), *Light::getDescriptorSetLayout(), *Light::getShadowMapDescriptorSetLayout()};
    VkDescriptorSetLayout lightPipelineLayouts[1] = {*Mesh::getDescriptorSetLayout()};
    graphicsPipeline = Pipeline::createDrawingPipeline("../shaders/obj_vert.spv", "../shaders/obj_frag.spv", swapchain, &renderPass, graphicPipelineLayouts, 3, 0);
    lightPipeline = Pipeline::createDrawingPipeline("../shaders/light_vert.spv", "../shaders/light_frag.spv", swapchain, &renderPass, lightPipelineLayouts, 1, 0);
    beginGraphicsCommandBuffer();
    framebuffer = new Framebuffer(swapchain, &renderPass);
    Light::recreateAllLights(swapchain->swapchainImages.size());
    Object::recreateAllObjects(swapchain->swapchainImages.size());
    endGraphicsCommandBuffer();
    allocateCommandBuffers();
}

void Graphics::querySwapchainSupport(VkPhysicalDevice testDevice, SwapchainSupportDetails *details)
{
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(testDevice, surface, &details->capabilities);
    uint32_t formatCount;
    vkGetPhysicalDeviceSurfaceFormatsKHR(testDevice, surface, &formatCount, nullptr);

    if (formatCount != 0)
    {
        details->formats.resize(formatCount);
        vkGetPhysicalDeviceSurfaceFormatsKHR(testDevice, surface, &formatCount,
                                             details->formats.data());
    }

    uint32_t presentModeCount;
    vkGetPhysicalDeviceSurfacePresentModesKHR(testDevice, surface, &presentModeCount, nullptr);

    if (presentModeCount != 0)
    {
        details->presentModes.resize(presentModeCount);
        vkGetPhysicalDeviceSurfacePresentModesKHR(testDevice, surface, &presentModeCount,
                                                  details->presentModes.data());
    }
    // return &details;
}

VkDevice Graphics::getDevice()
{
    return device;
}

Swapchain *Graphics::getSwapchain()
{
    return swapchain;
}

void Graphics::getSwapchainSupport(SwapchainSupportDetails *details)
{
    details = swapchainSupport;
}

Framebuffer *Graphics::getFramebuffer()
{
    return framebuffer;
}

size_t Graphics::getMaxFramesInFlight()
{
    return MAX_FRAMES_IN_FLIGHT;
}

Camera *Graphics::getCamera()
{
    return cam;
}

VkInstance Graphics::getInstance()
{
    return instance;
}

Pipeline *Graphics::getGraphicsPipeline()
{
    return graphicsPipeline;
}

Pipeline *Graphics::getLightPipeline()
{
    return lightPipeline;
}

VkCommandBuffer Graphics::getGraphicsCommandBuffer()
{
    return graphicsCommandBuffer;
}

VkCommandBuffer Graphics::getTransferCommandBuffer()
{
    return transferCommandBuffer;
}

std::vector<VkCommandBuffer> Graphics::getDrawCommandBuffer()
{
    return drawCommandBuffers;
}

VkQueue Graphics::getGraphicsQueue()
{
    return graphicsQueue;
}

VkQueue Graphics::getPresentQueue()
{
    return presentQueue;
}

VkQueue Graphics::getTransferQueue()
{
    return transferQueue;
}

VkCommandPool Graphics::getGraphicsCommandPool()
{
    return graphicsCommandPool;
}

VkCommandPool Graphics::getTransferCommandPool()
{
    return transferCommandPool;
}

QueueFamilyIndices Graphics::getQueueFamilyIndices()
{
    return indices;
}

VkSurfaceKHR Graphics::getSurface()
{
    return surface;
}

VkSampleCountFlagBits Graphics::getMsaaSamples()
{
    return msaaSamples;
}

VkPhysicalDevice Graphics::getPhysicalDevice()
{
    return physicalDevice;
}
