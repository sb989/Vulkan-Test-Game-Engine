#include "vtge_graphics.hpp"
#include "vtge_debug_helper_functions.hpp"
#include <array>
#include <set>
#include<iostream>
#include<memory>
#include<cstdlib>
#include<chrono>
#include <glm/glm.hpp>
#include<glm/gtx/hash.hpp>
#include<glm/gtc/quaternion.hpp>
#include<glm/gtx/quaternion.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "glm/gtx/string_cast.hpp"
#include <glm/gtx/norm.hpp>
#include <type_traits>
#include "vtge_swapchain.hpp"
#include "vtge_framebuffer.hpp"
#include "vtge_pipeline.hpp"
#include "vtge_object.hpp"
#include "vtge_light.hpp"
#define GLFW_INCLUDE_VULKAN
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define STB_IMAGE_IMPLEMENTATION
#define TINYOBJLOADER_IMPLEMENTATION
#define GLM_ENABLE_EXPERIMENTAL

VkDevice                        device;
VkSampleCountFlagBits           msaaSamples = VK_SAMPLE_COUNT_1_BIT;
VkQueue                         graphicsQueue, presentQueue, transferQueue;
VkCommandPool                   graphicsCommandPool, transferCommandPool;
QueueFamilyIndices              indices;
VkCommandBuffer                 transferCommandBuffer, graphicsCommandBuffer;
VkPhysicalDevice                physicalDevice = VK_NULL_HANDLE;
//VkDescriptorSetLayout           descriptorSetLayout;
extern bool                     enableValidationLayers;

Graphics::Graphics(uint32_t width, uint32_t height, std::string windowTitle){
        WIDTH = width;
        HEIGHT = height;
        this->windowTitle = windowTitle;
        setUpWindow();
        setUpGraphics();
        camXPos = camYPos = camZPos = 0.0f;
        camPos = glm::vec3(camXPos, camYPos, camZPos);
        lookDir = glm::vec3(0.0f, -100.0f, 0.0f);
        viewMat = glm::lookAt(glm::vec3(camXPos, camYPos, camZPos), lookDir + camPos, glm::vec3(0.0f, 0.0f, -1.0f));
        camYaw = camPitch = oldCamPitch = oldCamYaw = 0.0f;
        projectionMat = glm::perspective(glm::radians(45.0f), swapchain->swapchainExtent.width / (float) swapchain->swapchainExtent.height,  0.1f, 400.0f);
}

Graphics::~Graphics(){
    std::cout<<"cleanup swapchain"<<std::endl;
    cleanupSwapchain();
    Object::destroyDescriptorSetLayout();
    Light::destroyDescriptorSetLayout();
    std::cout<<"delete model list"<<std::endl;
    Object::destroyAllObjects();
    Light::destroyAllLights();
    std::cout<<"done"<<std::endl;
    for (size_t i = 0; i<MAX_FRAMES_IN_FLIGHT; i++){
        vkDestroySemaphore(device, renderFinishedSemaphores[i], nullptr);
        vkDestroySemaphore(device, imageAvailableSemaphores[i], nullptr);
        vkDestroyFence(device, inFlightFences[i], nullptr);
    }
    vkDestroyCommandPool(device, graphicsCommandPool, nullptr);
    vkDestroyCommandPool(device, transferCommandPool, nullptr);            
    vkDestroyDevice(device, nullptr);
    if (enableValidationLayers) {
        debug::DestroyDebugUtilsMessengerEXT(instance, &debugMessenger, nullptr);
    }
    vkDestroySurfaceKHR(instance, surface, nullptr);
    vkDestroyInstance(instance, nullptr);
    glfwDestroyWindow(window);
    glfwTerminate();
}

void Graphics::setUpWindow(){
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    //glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
    window = glfwCreateWindow(WIDTH , HEIGHT, windowTitle.c_str(), nullptr, nullptr);
    glfwSetWindowUserPointer(window, this);
    // glfwSetInputMode(window,GLFW_CURSOR,GLFW_CURSOR_DISABLED);
    // if(glfwRawMouseMotionSupported()){
    //     glfwSetInputMode(window,GLFW_RAW_MOUSE_MOTION, GLFW_TRUE);
    // }
    cursorXPos = cursorYPos = 0.0f;
    //glfwSetCursorPos(window, cursorXPos, cursorYPos);
    glfwSetFramebufferSizeCallback(window, framebufferResizeCallback);
}

void Graphics::setUpGraphics(){
    createInstance();
    debug::setupDebugMessenger(instance, &debugMessenger);
    createSurface();
    pickPhysicalDevice();
    createLogicalDevice();
    swapchain = new Swapchain(&surface, window, swapchainSupport);
    createRenderPass();
    createCommandPool();
    graphicsCommandBuffer = beginSingleTimeCommands(graphicsCommandPool);
    transferCommandBuffer = beginSingleTimeCommands(transferCommandPool);
    std::cout<<"creating framebuffer now!"<<std::endl;
    framebuffer = new Framebuffer(swapchain, &renderPass);
    std::cout<<"finished creating framebuffer creating model now!"<<std::endl;
    std::cout<<"creating objects"<<std::endl;
    createLight("../models/cube.obj", glm::vec3(1.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1,1,1), glm::vec3(15, 5, 1));
    createLight("../models/cube.obj", glm::vec3(1.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1,0,0), glm::vec3(15, 5, 1));

    createObject(VIKING_MODEL_PATH,VIKING_TEXTURE_PATH, glm::vec3(-4, 5, 1), glm::vec3(1.0f), glm::vec3(0.0f));
    createObject(BANANA_MODEL_PATH, BANANA_TEXTURE_PATH, glm::vec3(2, 5, -1), glm::vec3(0.05f), glm::vec3(0.0f, 0.0f, 0.0f));
    createObject("../models/cube.obj", "../textures/banana.jpg", glm::vec3(2, 10, -1), glm::vec3(1.0f), glm::vec3(0.0f, 0.0f, 0.0f));
    createObject(BANANA_MODEL_PATH, BANANA_TEXTURE_PATH, glm::vec3(2, -5, -1), glm::vec3(0.05f), glm::vec3(0.0f, 90.0f, 0.0f));
    graphicsPipeline = new Pipeline("../shaders/obj_vert.spv", "../shaders/obj_frag.spv", swapchain, &renderPass, Object::getDescriptorSetLayout());
    lightPipeline = new Pipeline("../shaders/light_vert.spv", "../shaders/light_frag.spv", swapchain, &renderPass, Light::getDescriptorSetLayout());
    std::cout<<"finisehd creating objects"<<std::endl;
    std::cout<<"finisehd creating models!"<<std::endl;
    
    endSingleTimeCommands(transferCommandBuffer, transferCommandPool, transferQueue);
    std::cout<<"submitting transfer command buffer!"<<std::endl;
    endSingleTimeCommands(graphicsCommandBuffer, graphicsCommandPool, graphicsQueue);
    std::cout<<"submitting grahpics command buffer!"<<std::endl;
    buffer::cleanupStagingBuffers();
    allocateDrawCommandBuffers();
    //createDrawCommandBuffers();
    std::cout<<"finished creating draw command buffers!"<<std::endl;
    createSyncObjects();
}

void Graphics::createInstance() {
    if (enableValidationLayers && !getterChecker::checkValidationLayerSupport(validationLayers)) {
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
    const char** glfwExtensions;
    glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
    auto extensions = getterChecker::getRequiredExtensions();
    createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
    createInfo.ppEnabledExtensionNames = extensions.data();
    VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo{};
    if (enableValidationLayers) {
        createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
        createInfo.ppEnabledLayerNames = validationLayers.data();

        debug::populateDebugMessengerCreateInfo(debugCreateInfo);
        createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*) &debugCreateInfo;
    } else {
        createInfo.enabledLayerCount = 0;
        createInfo.pNext = nullptr;
    }
    if (vkCreateInstance(&createInfo, nullptr, &instance) != VK_SUCCESS) {
        throw std::runtime_error("failed to create instance!");
    }
    uint32_t extensionCount = 0;
    vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);
    std::vector<VkExtensionProperties> exts(extensionCount);
    vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, exts.data());
    std::cout << "available extensions:\n";
    for (const auto& extension : exts) {
        std::cout << '\t' << extension.extensionName << '\n';
    }
}

void Graphics::createSurface(){
    if (glfwCreateWindowSurface(instance, window, nullptr, &surface) != VK_SUCCESS) {
        throw std::runtime_error("failed to create window surface!");
    }
}

void Graphics::pickPhysicalDevice(){
    uint32_t deviceCount = 0;
    vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);

    if(deviceCount == 0){
        throw std::runtime_error("failed to find GPUs with Vulkan support!");
    }
    std::cout<<"device count is "<<deviceCount<<std::endl;
    std::vector<VkPhysicalDevice> devices(deviceCount);
    vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data());
    for (const auto& device : devices){
        swapchainSupport = querySwapchainSupport(device);
        indices = findQueueFamilies(device);
        if(getterChecker::isDeviceSuitable(device, swapchainSupport, deviceExtensions)){
            physicalDevice = device;
            msaaSamples = getterChecker::getMaxUsableSampleCount();
            break;
        }
    }

    if(physicalDevice == VK_NULL_HANDLE){
        throw std::runtime_error("failed to find a suitable GPU!");
    }
}


void Graphics::createLogicalDevice(){

    std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
    std::set<uint32_t> uniqueQueueFamilies = {indices.graphicsFamily.value(),
    indices.presentFamily.value(), indices.transferFamily.value()};
    float queuePriority = 1.0f;

    for(uint32_t queueFamily : uniqueQueueFamilies) {
        VkDeviceQueueCreateInfo queueCreateInfo{};
        queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queueCreateInfo.queueFamilyIndex = queueFamily;
        queueCreateInfo.queueCount = 1;
        queueCreateInfo.pQueuePriorities = &queuePriority;
        queueCreateInfos.push_back(queueCreateInfo);
    }
    
    VkPhysicalDeviceFeatures deviceFeatures {};
    deviceFeatures.samplerAnisotropy = VK_TRUE;
    VkDeviceCreateInfo createInfo {};
    createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    createInfo.pQueueCreateInfos = queueCreateInfos.data();
    createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
    createInfo.pEnabledFeatures = &deviceFeatures;
    createInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size());
    createInfo.ppEnabledExtensionNames = deviceExtensions.data();
    if (enableValidationLayers){
        createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
        createInfo.ppEnabledLayerNames = validationLayers.data();
    } else {
        createInfo.enabledLayerCount = 0;
    }

    if (vkCreateDevice(physicalDevice, &createInfo, nullptr, &device) != VK_SUCCESS) {
    throw std::runtime_error("failed to create logical device!");
    }

    vkGetDeviceQueue(device, indices.graphicsFamily.value(), 0, &graphicsQueue);
    vkGetDeviceQueue(device, indices.presentFamily.value(), 0, &presentQueue);
    vkGetDeviceQueue(device,indices.transferFamily.value(), 0, &transferQueue);
}

void Graphics::createRenderPass(){
    VkAttachmentDescription depthAttachment{};
    depthAttachment.format = getterChecker::findDepthFormat();
    depthAttachment.samples = msaaSamples;
    depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
    VkAttachmentReference depthAttachmentRef{};
    depthAttachmentRef.attachment = 1;
    depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    VkAttachmentDescription colorAttachment{};
    colorAttachment.format = swapchain->swapchainImageFormat;
    colorAttachment.samples = msaaSamples;
    colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    colorAttachment.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    VkAttachmentReference colorAttachmentRef{};
    colorAttachmentRef.attachment = 0;
    colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkAttachmentDescription colorAttachmentResolve{};
    colorAttachmentResolve.format = swapchain->swapchainImageFormat;
    colorAttachmentResolve.samples = VK_SAMPLE_COUNT_1_BIT;
    colorAttachmentResolve.loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    colorAttachmentResolve.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    colorAttachmentResolve.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    colorAttachmentResolve.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    colorAttachmentResolve.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    colorAttachmentResolve.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    VkAttachmentReference colorAttachmentResolveRef{};
    colorAttachmentResolveRef.attachment = 2;
    colorAttachmentResolveRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkSubpassDependency dependency{};
    dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    dependency.dstSubpass = 0;
    dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    dependency.srcAccessMask = 0;
    dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

    VkSubpassDescription subpass{};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &colorAttachmentRef;
    subpass.pDepthStencilAttachment = &depthAttachmentRef;
    subpass.pResolveAttachments = &colorAttachmentResolveRef;
    std::array<VkAttachmentDescription, 3> attachments = {colorAttachment, depthAttachment, colorAttachmentResolve};

    VkRenderPassCreateInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
    renderPassInfo.pAttachments = attachments.data();
    renderPassInfo.subpassCount = 1;
    renderPassInfo.pSubpasses = &subpass;
    renderPassInfo.dependencyCount = 1;
    renderPassInfo.pDependencies = &dependency;
    if (vkCreateRenderPass(device, &renderPassInfo, nullptr, &renderPass) != VK_SUCCESS) {
        throw std::runtime_error("failed to create render pass!");
    }
}

void Graphics::createCommandPool() {
    VkCommandPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolInfo.queueFamilyIndex = indices.graphicsFamily.value();
    poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT; // Optional
    if (vkCreateCommandPool(device, &poolInfo, nullptr, &graphicsCommandPool) != VK_SUCCESS) {
        throw std::runtime_error("failed to create command pool!");
    }
    poolInfo.queueFamilyIndex = indices.transferFamily.value();
    // this flag should be used for pools that have shortlived buffers
    poolInfo.flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT;
    if (vkCreateCommandPool(device, &poolInfo, nullptr, &transferCommandPool) != VK_SUCCESS){
        throw std::runtime_error("failed to create command pool for transfer queue!");
    }
}


void Graphics::allocateDrawCommandBuffers(){
    drawCommandBuffers.resize(framebuffer->swapchainFramebuffers.size());
    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool = graphicsCommandPool;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount = (uint32_t) drawCommandBuffers.size();

    if (vkAllocateCommandBuffers(device, &allocInfo, drawCommandBuffers.data()) != VK_SUCCESS) {
        throw std::runtime_error("failed to allocate command buffers!");
    }
    for (size_t i = 0; i < drawCommandBuffers.size(); i++) {
        populateDrawCommandBuffer(i);
    }
}

void Graphics::populateDrawCommandBuffer(size_t index) {
    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = 0; // Optional
    beginInfo.pInheritanceInfo = nullptr; // Optional

    if (vkBeginCommandBuffer(drawCommandBuffers[index], &beginInfo) != VK_SUCCESS) {
        throw std::runtime_error("failed to begin recording command buffer!");
    }
    VkRenderPassBeginInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassInfo.renderPass = renderPass;
    renderPassInfo.framebuffer = framebuffer->swapchainFramebuffers[index];
    renderPassInfo.renderArea.offset = {0, 0};
    renderPassInfo.renderArea.extent = swapchain->swapchainExtent;
    std::array<VkClearValue, 2> clearValues{};
    clearValues[0].color = {{0.0f, 0.0f, 0.0f, 1.0f}};
    clearValues[1].depthStencil = {1.0f, 0};
    renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
    renderPassInfo.pClearValues = clearValues.data();
    vkCmdBeginRenderPass(drawCommandBuffers[index], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
    //PushConstants push;
    //push.normMatrix = ;
    //vkCmdPushConstants(drawCommandBuffers[index], pipelineLayout, VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(PushConstants), &push);
    vkCmdBindPipeline(drawCommandBuffers[index], VK_PIPELINE_BIND_POINT_GRAPHICS, *graphicsPipeline->getPipeline());
    Object::drawAllObjects(&drawCommandBuffers[index], graphicsPipeline, index);
    Light::drawAllLights(&drawCommandBuffers[index], lightPipeline, index);
    vkCmdEndRenderPass(drawCommandBuffers[index]);
    if (vkEndCommandBuffer(drawCommandBuffers[index]) != VK_SUCCESS) {
        throw std::runtime_error("failed to record command buffer!");
    }
    
}

void Graphics::createSyncObjects(){
    imageAvailableSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
    renderFinishedSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
    inFlightFences.resize(MAX_FRAMES_IN_FLIGHT);
    imagesInFlight.resize(swapchain->swapchainImages.size(), VK_NULL_HANDLE);
    VkSemaphoreCreateInfo semaphoreInfo{};
    semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
    VkFenceCreateInfo fenceInfo {};
    fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;
    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++){
        if (vkCreateSemaphore(device, &semaphoreInfo, nullptr, &imageAvailableSemaphores[i]) 
        != VK_SUCCESS ||
            vkCreateSemaphore(device, &semaphoreInfo, nullptr, &renderFinishedSemaphores[i]) 
            != VK_SUCCESS ||
            vkCreateFence(device, &fenceInfo, nullptr, &inFlightFences[i]) != VK_SUCCESS
        ) {
            throw std::runtime_error("failed to create sync objects for a frame!");
        }
    }
}

bool Graphics::waitForFence(uint32_t &imageIndex){
    vkWaitForFences(device, 1, &inFlightFences[currentFrame], VK_TRUE, UINT64_MAX);
    //vkResetFences(device, 1, &inFlightFences[currentFrame]);
    //uint32_t imageIndex;
    VkResult result = vkAcquireNextImageKHR(device, swapchain->swapchain, UINT64_MAX,
        imageAvailableSemaphores[currentFrame], VK_NULL_HANDLE, &imageIndex);

    if (result == VK_ERROR_OUT_OF_DATE_KHR) {
        recreateSwapchain();
        return false;
    } else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
        throw std::runtime_error("failed to acquire swap chain image!");
    }
    
    if(imagesInFlight[imageIndex] != VK_NULL_HANDLE){
            vkWaitForFences(device, 1, &imagesInFlight[imageIndex], VK_TRUE, UINT64_MAX);
    }
    imagesInFlight[imageIndex] = inFlightFences[currentFrame];
    return true;
}

void Graphics::submitQueue(VkSemaphore signalSemaphores[], uint32_t imageIndex){
    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    VkSemaphore waitSemaphores[] = {imageAvailableSemaphores[currentFrame]};
    VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = waitSemaphores;
    submitInfo.pWaitDstStageMask = waitStages;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &drawCommandBuffers[imageIndex];
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = signalSemaphores;
    vkResetFences(device, 1, &inFlightFences[currentFrame]);
    if (vkQueueSubmit(graphicsQueue, 1, &submitInfo, inFlightFences[currentFrame]) != VK_SUCCESS) {
        throw std::runtime_error("failed to submit draw command buffer!");
    }
}

void Graphics::presentQueueToScreen(uint32_t &imageIndex, VkSemaphore signalSemaphores[]){
    VkPresentInfoKHR presentInfo{};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = signalSemaphores;
    VkSwapchainKHR swapchains[] = {swapchain->swapchain};
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = swapchains;
    presentInfo.pImageIndices = &imageIndex;
    presentInfo.pResults = nullptr; // Optional
    VkResult result = vkQueuePresentKHR(presentQueue, &presentInfo);
    if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || framebufferResized) {
        framebufferResized = false;
        recreateSwapchain();
    } else if (result != VK_SUCCESS) {
        throw std::runtime_error("failed to present swap chain image!");
    }
}

void Graphics::drawFrame(){
    uint32_t imageIndex;
    waitForFence(imageIndex);
    vkResetCommandBuffer(drawCommandBuffers[imageIndex], 0);
    
    handleKeyPress(window);
    float x = cos(glfwGetTime()/4.0f)/4.0f;
    float y = sin(glfwGetTime()/4.0f)/4.0f;
    glm::vec3 displacement(x,y,0.0f);
    Model * m = Light::getLight(0)->getModel();
    m->moveModel(displacement);
    if(glfwGetInputMode(window, GLFW_CURSOR) == GLFW_CURSOR_DISABLED)
        handleMouse(window);
    updateCamera();
    Object::updateAllObjects(imageIndex, projectionMat, viewMat);
    Light::updateAllLights(imageIndex, projectionMat, viewMat);
    populateDrawCommandBuffer(imageIndex);
    VkSemaphore signalSemaphores[] = {renderFinishedSemaphores[currentFrame]};
    submitQueue(signalSemaphores, imageIndex);
    presentQueueToScreen(imageIndex, signalSemaphores);
    currentFrame = (currentFrame + 1)% MAX_FRAMES_IN_FLIGHT;
}

void Graphics::handleKeyPress(GLFWwindow* window){
    int up_state = glfwGetKey(window, GLFW_KEY_W);
    int down_state = glfwGetKey(window, GLFW_KEY_S);
    int left_state = glfwGetKey(window, GLFW_KEY_A);
    int right_state = glfwGetKey(window, GLFW_KEY_D);
    int esc_state = glfwGetKey(window, GLFW_KEY_ESCAPE);
    int enter_state = glfwGetKey(window, GLFW_KEY_ENTER);
    int space_state = glfwGetKey(window, GLFW_KEY_SPACE);
    int shift_state = glfwGetKey(window, GLFW_KEY_LEFT_SHIFT);
    
    if(up_state == GLFW_PRESS || up_state == GLFW_REPEAT && down_state != GLFW_REPEAT){
        camXPos -= .3f * lookDir.x;
        camYPos -= .3f * lookDir.y;
    } else if(down_state == GLFW_PRESS || down_state == GLFW_REPEAT && up_state != GLFW_REPEAT){
        camXPos += .3f * lookDir.x;
        camYPos += .3f * lookDir.y;
    }

    if(left_state == GLFW_PRESS || left_state == GLFW_REPEAT && right_state != GLFW_REPEAT){
        camXPos += .3f * lookDir.y;
        camYPos -= .3f * lookDir.x;
    } else if(right_state == GLFW_PRESS || right_state == GLFW_REPEAT && left_state != GLFW_REPEAT){
        camXPos -= .3f * lookDir.y;
        camYPos += .3f * lookDir.x;
    }    

    if(esc_state == GLFW_PRESS || esc_state == GLFW_REPEAT && enter_state!= GLFW_REPEAT){
        glfwSetInputMode(window,GLFW_CURSOR,GLFW_CURSOR_NORMAL);
        glfwSetInputMode(window,GLFW_RAW_MOUSE_MOTION, GLFW_FALSE);
    } else if(enter_state == GLFW_PRESS || enter_state == GLFW_REPEAT && esc_state != GLFW_REPEAT){
        glfwSetInputMode(window,GLFW_CURSOR,GLFW_CURSOR_DISABLED);
        if(glfwRawMouseMotionSupported()){
            glfwSetInputMode(window,GLFW_RAW_MOUSE_MOTION, GLFW_TRUE);
        }
        glfwSetCursorPos(window, 0, 0);
    }

    if(space_state == GLFW_PRESS || space_state == GLFW_REPEAT && shift_state != GLFW_REPEAT){
        camZPos += .3f;
    } else if(shift_state == GLFW_PRESS || shift_state == GLFW_REPEAT && space_state != GLFW_REPEAT){
        camZPos -= .3f;
    }   
}

void Graphics::handleMouse(GLFWwindow* window){
    double x_pos, y_pos;
    glfwGetCursorPos(window, &x_pos, &y_pos);
    double xdiff = x_pos - cursorXPos;
    double ydiff = y_pos - cursorYPos;
    oldCamYaw = camYaw;
    oldCamPitch = camPitch;
    camYaw += xdiff;
    camPitch += ydiff;
    cursorXPos = x_pos;
    cursorYPos = y_pos;
    
}

void Graphics::updateCamera(){
    //i used quaterions becuz i was curious how they worked
    //probably could have done it with less code if i didnt create my own lookat matrix
    //or if i didnt use quaterions
    glm::quat rotateQuat;
    if(lookDir.z<= -0.9 && camPitch < oldCamPitch || lookDir.z>= 0.9 && camPitch > oldCamPitch){
        camPitch = oldCamPitch;
    }
    glm::vec3 worldup = glm::vec3(0.0f,0.0f,1.0f);
    rotateQuat = glm::angleAxis(glm::radians(oldCamPitch - camPitch), glm::cross(worldup, lookDir));
    rotateQuat = glm::angleAxis(glm::radians(oldCamYaw - camYaw), worldup) * rotateQuat;
    camPos = glm::vec3(camXPos, camYPos, camZPos);
    lookDir = rotateQuat * lookDir * glm::inverse(rotateQuat);
    lookDir = glm::normalize(lookDir);

    glm::vec3 newCamRight = glm::normalize(glm::cross(worldup, lookDir));
    glm::vec3 newCamUp = glm::normalize(glm::cross(newCamRight, lookDir));
    viewMat = glm::mat4(
        glm::vec4(newCamRight[0], newCamUp[0], lookDir[0], 0.0f), 
        glm::vec4(newCamRight[1], newCamUp[1], lookDir[1], 0.0f),
        glm::vec4(newCamRight[2], newCamUp[2], lookDir[2], 0.0f),
        glm::vec4(-glm::dot(camPos, newCamRight), -glm::dot(camPos, newCamUp), -glm::dot(camPos, lookDir), 1.0f));
}

QueueFamilyIndices Graphics::findQueueFamilies(VkPhysicalDevice device) {
    QueueFamilyIndices ind;
    // Assign index to queue families that could be found
    uint32_t queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);
    std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

    int i = 0;
    for (const auto& queueFamily : queueFamilies) {
        VkBool32 presentSupport = false;
        vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &presentSupport);
        if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
            ind.graphicsFamily = i;
        } else if (queueFamily.queueFlags & VK_QUEUE_TRANSFER_BIT) {
            ind.transferFamily = i;
        }
        if(presentSupport){
            ind.presentFamily = i;
        }
        if (ind.isComplete()){
            break;
        }
        i++;
    }
    if(!ind.transferFamily.has_value()){
        ind.transferFamily = ind.graphicsFamily;
    }
    return ind;
}    

void Graphics::framebufferResizeCallback(GLFWwindow* window, int width, int height){
    auto app = reinterpret_cast<Graphics*>(glfwGetWindowUserPointer(window));
    app->framebufferResized = true;
}

void Graphics::createObject(std::string modelPath, std::string texturePath, glm::vec3 translate, glm::vec3 scale, glm::vec3 rotate){
    Object *obj = new Object(modelPath, texturePath, swapchain->swapchainImages.size());
    obj->getModel()->moveModel(translate);
    obj->getModel()->rotateModel(rotate);
    obj->getModel()->scaleModel(scale);
    obj->getModel()->setRotation(rotate/40.0f);
    
}

void Graphics::createLight(std::string modelPath, glm::vec3 scale, glm::vec3 rotate, glm::vec3 lightColor, glm::vec3 lightPos){
    Light *l = new Light(modelPath, lightColor, lightPos, swapchain->swapchainImages.size());
    l->getModel()->rotateModel(rotate);
    l->getModel()->scaleModel(scale);
    l->getModel()->setRotation(rotate/40.0f);
    //lightList.push_back(m);
}

VkCommandBuffer Graphics::beginSingleTimeCommands(VkCommandPool pool){
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

void Graphics::endSingleTimeCommands(VkCommandBuffer commandBuffer, VkCommandPool pool, VkQueue queue){
    vkEndCommandBuffer(commandBuffer);
    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffer;
    vkQueueSubmit(queue, 1, &submitInfo, VK_NULL_HANDLE);
    vkQueueWaitIdle(queue);
    vkFreeCommandBuffers(device, pool, 1, &commandBuffer);
}

void Graphics::cleanupSwapchain(){
    delete framebuffer;
    vkFreeCommandBuffers(device, graphicsCommandPool, static_cast<uint32_t>(drawCommandBuffers.size()), drawCommandBuffers.data());
    delete graphicsPipeline;
    delete lightPipeline;
    vkDestroyRenderPass(device, renderPass, nullptr);
    Object::cleanupAllMemory();
    Light::cleanupAllMemory();
    //Model::destroyLightBufferAndMemory(swapchain->swapchainImages.size());
    delete swapchain;
}

void Graphics::recreateSwapchain(){
    std::cout<<"recreating swapchain"<<std::endl;
    int width = 0, height = 0;
    glfwGetFramebufferSize(window, &width, &height);
    while (width == 0 || height == 0){
        glfwGetFramebufferSize(window, &width, &height);
        glfwWaitEvents();
    }
    vkDeviceWaitIdle(device);
    cleanupSwapchain();
    swapchainSupport = querySwapchainSupport(physicalDevice);

    swapchain = new Swapchain(&surface, window, swapchainSupport);

    createRenderPass();
    graphicsPipeline = new Pipeline("../shaders/obj_vert.spv", "../shaders/obj_frag.spv", swapchain, &renderPass, Object::getDescriptorSetLayout());
    lightPipeline = new Pipeline("../shaders/light_vert.spv", "../shaders/light_frag.spv", swapchain, &renderPass, Light::getDescriptorSetLayout());
    graphicsCommandBuffer = beginSingleTimeCommands(graphicsCommandPool);
    framebuffer = new Framebuffer(swapchain, &renderPass);
    Light::recreateAllLights(swapchain->swapchainImages.size());
    Object::recreateAllObjects(swapchain->swapchainImages.size());
    endSingleTimeCommands(graphicsCommandBuffer, graphicsCommandPool, graphicsQueue);
    allocateDrawCommandBuffers();
    //createDrawCommandBuffers();
}

SwapchainSupportDetails Graphics::querySwapchainSupport(VkPhysicalDevice testDevice) {
        SwapchainSupportDetails details;
        vkGetPhysicalDeviceSurfaceCapabilitiesKHR(testDevice, surface, &details.capabilities);
        uint32_t formatCount;
        vkGetPhysicalDeviceSurfaceFormatsKHR(testDevice, surface, &formatCount, nullptr);

        if(formatCount !=0){
            details.formats.resize(formatCount);
            vkGetPhysicalDeviceSurfaceFormatsKHR(testDevice, surface, &formatCount, 
            details.formats.data());
        }

        uint32_t presentModeCount;
        vkGetPhysicalDeviceSurfacePresentModesKHR(testDevice, surface, &presentModeCount, nullptr);

        if(presentModeCount != 0){
            details.presentModes.resize(presentModeCount);
            vkGetPhysicalDeviceSurfacePresentModesKHR(testDevice, surface, &presentModeCount, 
            details.presentModes.data());
        }
        return details;
}

glm::quat Graphics::angleBetweenVectors(glm::vec3 start, glm::vec3 end){
    //http://www.opengl-tutorial.org/intermediate-tutorials/tutorial-17-quaternions/
    start = glm::normalize(start);
    end = glm::normalize(end);
    float cosTheta = glm::dot(start,end);
    glm::vec3 rotationAxis;
    if (cosTheta < -1 + 0.001f){
		// special case when vectors in opposite directions:
		// there is no "ideal" rotation axis
		// So guess one; any will do as long as it's perpendicular to start
		rotationAxis = glm::cross(glm::vec3(0.0f, 0.0f, 1.0f), start);
		if (glm::length2(rotationAxis) < 0.01 ) // bad luck, they were parallel, try again!
			rotationAxis = glm::cross(glm::vec3(1.0f, 0.0f, 0.0f), start);

		rotationAxis = normalize(rotationAxis);
		return glm::angleAxis(glm::radians(180.0f), rotationAxis);
	}

	rotationAxis = glm::cross(start, end);

	float s = sqrt( (1+cosTheta)*2 );
	float invs = 1 / s;

	return glm::quat(
		s * 0.5f, 
		rotationAxis.x * invs,
		rotationAxis.y * invs,
		rotationAxis.z * invs
	);
}