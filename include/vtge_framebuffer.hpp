#ifndef __VTGE_FRAMEBUFFER_HPP__
#define __VTGE_FRAMEBUFFER_HPP__
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <vector>
class Swapchain;

class Framebuffer
{
public:
     Framebuffer(Swapchain *swapchain, VkRenderPass *renderPass);
     Framebuffer(VkRenderPass *renderPass, uint32_t count);
     ~Framebuffer();

     std::vector<VkFramebuffer> framebuffers;
     std::vector<VkImageView> getShadowDepthImageView();
     VkImageView getCombinedShadowImageView();

private:
     std::vector<VkImageView> depthImageView, colorImageView, shadowDepthImageView;
     VkImageView combinedShadowImageView;
     VkImage depthImage, colorImage, shadowDepthImage;
     VkDeviceMemory depthImageMemory, colorImageMemory, shadowDepthImageMemory;
     VkExtent2D swapchainExtent;
     VkFormat swapchainImageFormat;
     VkRenderPass *renderPass;
     uint32_t framebufferCount;

     void createFramebuffers(std::vector<VkImageView> attachments, uint32_t width, uint32_t height, uint32_t layers);
     void createColorResources();
     void createDepthResources(VkImage *image, VkDeviceMemory *imageMemory, std::vector<VkImageView> *imageViews,
                               VkImageUsageFlags usage, float width, float height, uint32_t layers, VkImageViewType imageViewType,
                               VkSampleCountFlagBits sampleCount, VkImageLayout imageLayout);
     void createShadowDepthResources(uint32_t count);
     void createImageDepthResources();
};
#endif