#include "vtge_framebuffer.hpp"
#include "vtge_image.hpp"
#include<array>
#include <stdexcept>
#include "vtge_swapchain.hpp"
#include "vtge_shared_variables.hpp"
Framebuffer::Framebuffer(Swapchain *swapchain, 
    VkRenderPass *renderPass, VkQueue *graphicsQueue, VkCommandPool *graphicsCommandPool){
        this->swapchainExtent = swapchain->swapchainExtent;
        this->swapchainImageFormat = swapchain->swapchainImageFormat;
        this->swapchainImageViews = swapchain->swapchainImageViews;
        this->renderPass = renderPass;
        this->graphicsQueue = graphicsQueue;
        this->graphicsCommandPool = graphicsCommandPool;
        createColorResources();
        createDepthResources();
        createFramebuffers();
}

Framebuffer::~Framebuffer(){
    vkDestroyImageView(*sharedVariables::device, colorImageView, nullptr);
    vkDestroyImage(*sharedVariables::device, colorImage, nullptr);
    vkFreeMemory(*sharedVariables::device, colorImageMemory, nullptr);
    vkDestroyImageView(*sharedVariables::device, depthImageView, nullptr);
    vkDestroyImage(*sharedVariables::device, depthImage, nullptr);
    vkFreeMemory(*sharedVariables::device, depthImageMemory, nullptr);
    for (size_t i = 0; i < swapchainFramebuffers.size(); i++) {
        vkDestroyFramebuffer(*sharedVariables::device, swapchainFramebuffers[i], nullptr);
    }
}

void Framebuffer::createFramebuffers(){
    swapchainFramebuffers.resize(swapchainImageViews.size());
    for (size_t i = 0; i < swapchainImageViews.size(); i++) {
        std::array<VkImageView, 3> attachments = {
            colorImageView,
            depthImageView,
            swapchainImageViews[i]
        };
        VkFramebufferCreateInfo framebufferInfo{};
        framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebufferInfo.renderPass = *renderPass;
        framebufferInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
        framebufferInfo.pAttachments = attachments.data();
        framebufferInfo.width = swapchainExtent.width;
        framebufferInfo.height = swapchainExtent.height;
        framebufferInfo.layers = 1;
        if (vkCreateFramebuffer(*sharedVariables::device, &framebufferInfo, nullptr, &swapchainFramebuffers[i]) != VK_SUCCESS) {
            throw std::runtime_error("failed to create framebuffer!");
        }
    }
}


void Framebuffer::createColorResources(){
    VkFormat colorFormat = swapchainImageFormat;
    image::createImage(swapchainExtent.width, swapchainExtent.height, 1, sharedVariables::msaaSamples, colorFormat,
        VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT| VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, colorImage, colorImageMemory);
    colorImageView = image::createImageView(colorImage, colorFormat, VK_IMAGE_ASPECT_COLOR_BIT, 1);
}

void Framebuffer::createDepthResources(){
    VkFormat depthFormat = getterChecker::findDepthFormat(*physicalDevice);
    image::createImage(swapchainExtent.width, swapchainExtent.height, 1, msaaSamples, depthFormat, VK_IMAGE_TILING_OPTIMAL,
    VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, depthImage, depthImageMemory, *device);
    depthImageView = image::createImageView(depthImage, *device, depthFormat, VK_IMAGE_ASPECT_DEPTH_BIT, 1);
    graphicsCommandBuffer = beginSingleTimeCommands(graphicsCommandPool);
    image::transitionImageLayout(depthImage, depthFormat, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
        *graphicsCommandBuffer, *graphicsQueue, 1);
    endSingleTimeCommands(graphicsCommandBuffer, graphicsCommandPool, graphicsQueue);
}