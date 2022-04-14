#include "vtge_framebuffer.hpp"
#include "vtge_image.hpp"
#include "vtge_swapchain.hpp"
#include "vtge_getter_and_checker_functions.hpp"
#include "vtge_graphics.hpp"
#include <array>
#include <iostream>
#include <stdexcept>

Framebuffer::Framebuffer(Swapchain *swapchain, VkRenderPass *renderPass)
{
    // framebuffer for regular rendering
    this->swapchainExtent = swapchain->swapchainExtent;
    this->swapchainImageFormat = swapchain->swapchainImageFormat;
    std::vector<VkImageView> swapImgViews = swapchain->swapchainImageViews;
    this->renderPass = renderPass;
    this->framebufferCount = swapImgViews.size();
    createColorResources();
    createImageDepthResources();
    std::vector<VkImageView> attachments;
    attachments.push_back(colorImageView[0]);
    attachments.push_back(depthImageView[0]);
    createFramebuffers(attachments, swapchainExtent.width, swapchainExtent.height, 1);
}

Framebuffer::Framebuffer(VkRenderPass *renderPass, uint32_t count)
{
    // framebuffer for shadowmap
    this->renderPass = renderPass;
    createShadowDepthResources(count);
    this->framebufferCount = count;
    std::vector<VkImageView> attachments = {};
    createFramebuffers(attachments, getterChecker::getShadowMapWidth(), getterChecker::getShadowMapHeight(), count);
}

Framebuffer::~Framebuffer()
{
    VkDevice device = Graphics::getDevice();
    for (int i = 0; i < depthImageView.size(); i++)
        vkDestroyImageView(device, depthImageView[i], nullptr);
    vkDestroyImage(device, depthImage, nullptr);
    vkFreeMemory(device, depthImageMemory, nullptr);
    for (int i = 0; i < colorImageView.size(); i++)
        vkDestroyImageView(device, colorImageView[i], nullptr);
    vkDestroyImage(device, colorImage, nullptr);
    vkFreeMemory(device, colorImageMemory, nullptr);
    for (size_t i = 0; i < framebuffers.size(); i++)
    {
        vkDestroyFramebuffer(device, framebuffers[i], nullptr);
    }
}

void Framebuffer::createFramebuffers(std::vector<VkImageView> attachments, uint32_t width, uint32_t height, uint32_t layers)
{
    VkDevice device = Graphics::getDevice();

    framebuffers.resize(framebufferCount);
    if (attachments.size() == 2)
        attachments.resize(3);
    else if (attachments.size() == 0)
        attachments.resize(1);
    for (size_t i = 0; i < framebufferCount; i++)
    {
        if (attachments.size() == 3)
            attachments[2] = Graphics::getSwapchain()->swapchainImageViews[i];
        else if (attachments.size() == 1)
        {
            attachments[0] = shadowDepthImageView[i];
            // std::cout << "adding depth imageview to framebuffer" << std::endl;
        }
        VkFramebufferCreateInfo framebufferInfo{};
        framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebufferInfo.renderPass = *renderPass;
        framebufferInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
        framebufferInfo.pAttachments = attachments.data();
        framebufferInfo.width = width;
        framebufferInfo.height = height;
        framebufferInfo.layers = 1;
        if (vkCreateFramebuffer(device, &framebufferInfo, nullptr, &framebuffers[i]) != VK_SUCCESS)
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
    colorImageView.push_back(image::createImageView(colorImage, colorFormat, VK_IMAGE_ASPECT_COLOR_BIT, 1, 0, 1));
}

void Framebuffer::createDepthResources(VkImage *image, VkDeviceMemory *imageMemory, std::vector<VkImageView> *imageViews,
                                       VkImageUsageFlags usage, float width, float height, uint32_t layers, VkImageViewType imageViewType,
                                       VkSampleCountFlagBits sampleCount, VkImageLayout imageLayout)
{
    VkFormat depthFormat = getterChecker::findDepthFormat();
    VkCommandBuffer graphicsCommandBuffer = Graphics::getGraphicsCommandBuffer();
    image::createImage(width, height, 1, sampleCount, depthFormat, VK_IMAGE_TILING_OPTIMAL,
                       usage, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, *image, *imageMemory, layers);

    for (int count = 0; count < layers; count++)
    {
        VkImageView tempView = image::createImageView(*image, depthFormat, VK_IMAGE_ASPECT_DEPTH_BIT, 1, count, 1, imageViewType);
        imageViews->push_back(tempView);
    }
    image::transitionImageLayout(*image, depthFormat, VK_IMAGE_LAYOUT_UNDEFINED, imageLayout,
                                 graphicsCommandBuffer, 1, layers);
}

void Framebuffer::createImageDepthResources()
{
    VkSampleCountFlagBits msaaSamples = Graphics::getMsaaSamples();
    createDepthResources(&depthImage, &depthImageMemory, &depthImageView, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, (float)swapchainExtent.width,
                         (float)swapchainExtent.height, 1, VK_IMAGE_VIEW_TYPE_2D, msaaSamples, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);
}

void Framebuffer::createShadowDepthResources(uint32_t count)
{
    float height = getterChecker::getShadowMapHeight();
    float width = getterChecker::getShadowMapWidth();
    createDepthResources(
        &shadowDepthImage, &shadowDepthImageMemory, &shadowDepthImageView,
        VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, width, height, count, VK_IMAGE_VIEW_TYPE_2D_ARRAY,
        VK_SAMPLE_COUNT_1_BIT, VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL);

    combinedShadowImageView = image::createImageView(
        shadowDepthImage, getterChecker::findDepthFormat(),
        VK_IMAGE_ASPECT_DEPTH_BIT, 1, 0, count, VK_IMAGE_VIEW_TYPE_2D_ARRAY);
}

std::vector<VkImageView> Framebuffer::getShadowDepthImageView()
{
    return shadowDepthImageView;
}

VkImageView Framebuffer::getCombinedShadowImageView()
{
    return combinedShadowImageView;
}
