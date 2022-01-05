#include "vtge_framebuffer.hpp"
#include "vtge_image.hpp"
#include "vtge_swapchain.hpp"
#include "vtge_getter_and_checker_functions.hpp"
#include "vtge_graphics.hpp"
#include <array>
#include <stdexcept>

Framebuffer::Framebuffer(Swapchain *swapchain, VkRenderPass *renderPass)
{
    this->swapchainExtent = swapchain->swapchainExtent;
    this->swapchainImageFormat = swapchain->swapchainImageFormat;
    this->swapchainImageViews = swapchain->swapchainImageViews;
    this->renderPass = renderPass;
    createColorResources();
    createDepthResources();
    createFramebuffers();
}

Framebuffer::~Framebuffer()
{
    VkDevice device = Graphics::getDevice();
    vkDestroyImageView(device, depthImageView, nullptr);
    vkDestroyImage(device, depthImage, nullptr);
    vkFreeMemory(device, depthImageMemory, nullptr);
    vkDestroyImageView(device, colorImageView, nullptr);
    vkDestroyImage(device, colorImage, nullptr);
    vkFreeMemory(device, colorImageMemory, nullptr);
    for (size_t i = 0; i < swapchainFramebuffers.size(); i++)
    {
        vkDestroyFramebuffer(device, swapchainFramebuffers[i], nullptr);
    }
}

void Framebuffer::createFramebuffers()
{
    VkDevice device = Graphics::getDevice();

    swapchainFramebuffers.resize(swapchainImageViews.size());
    for (size_t i = 0; i < swapchainImageViews.size(); i++)
    {
        std::array<VkImageView, 3> attachments = {
            colorImageView,
            depthImageView,
            swapchainImageViews[i]};
        VkFramebufferCreateInfo framebufferInfo{};
        framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebufferInfo.renderPass = *renderPass;
        framebufferInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
        framebufferInfo.pAttachments = attachments.data();
        framebufferInfo.width = swapchainExtent.width;
        framebufferInfo.height = swapchainExtent.height;
        framebufferInfo.layers = 1;
        if (vkCreateFramebuffer(device, &framebufferInfo, nullptr, &swapchainFramebuffers[i]) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to create framebuffer!");
        }
    }
}

void Framebuffer::createColorResources()
{
    VkFormat colorFormat = swapchainImageFormat;
    VkSampleCountFlagBits msaaSamples = Graphics::getMsaaSamples();
    image::createImage(swapchainExtent.width, swapchainExtent.height, 1, msaaSamples, colorFormat,
                       VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
                       VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, colorImage, colorImageMemory);
    colorImageView = image::createImageView(colorImage, colorFormat, VK_IMAGE_ASPECT_COLOR_BIT, 1);
}

void Framebuffer::createDepthResources()
{
    VkFormat depthFormat = getterChecker::findDepthFormat();
    VkSampleCountFlagBits msaaSamples = Graphics::getMsaaSamples();
    VkCommandBuffer graphicsCommandBuffer = Graphics::getGraphicsCommandBuffer();
    image::createImage(swapchainExtent.width, swapchainExtent.height, 1, msaaSamples, depthFormat, VK_IMAGE_TILING_OPTIMAL,
                       VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, depthImage, depthImageMemory);
    depthImageView = image::createImageView(depthImage, depthFormat, VK_IMAGE_ASPECT_DEPTH_BIT, 1);
    image::transitionImageLayout(depthImage, depthFormat, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
                                 graphicsCommandBuffer, 1);
}