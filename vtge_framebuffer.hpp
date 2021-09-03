#ifndef __VTGE_FRAMEBUFFER_HPP__
#define __VTGE_FRAMEBUFFER_HPP__
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <vector>

class Framebuffer{
    std::vector<VkFramebuffer>      swapchainFramebuffers;
    VkImage                         depthImage, colorImage;
    VkDeviceMemory                  depthImageMemory, colorImageMemory;
    VkImageView                     depthImageView, colorImageView;
    std::vector<VkImageView>        swapchainImageViews;
    VkExtent2D                      swapchainExtent;
    VkFormat                        swapchainImageFormat;
    VkRenderPass                    *renderPass;
    VkQueue                         *graphicsQueue;
    VkCommandPool                   *graphicsCommandPool;
    void createFramebuffers();
    void createColorResources();
    void createDepthResources();
    Framebuffer(Swapchain *swapchain, 
    VkRenderPass *renderPass, VkQueue *graphicsQueue, 
    VkCommandPool *graphicsCommandPool);
};
#endif