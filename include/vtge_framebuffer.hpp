#ifndef __VTGE_FRAMEBUFFER_HPP__
#define __VTGE_FRAMEBUFFER_HPP__
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <vector>
class Swapchain;

class Framebuffer{
    public:
        Framebuffer(Swapchain *swapchain, VkRenderPass *renderPass);
        ~Framebuffer();
        std::vector<VkFramebuffer>      swapchainFramebuffers;
    private:
        VkImage                         depthImage, colorImage;
        VkDeviceMemory                  depthImageMemory, colorImageMemory;
        VkImageView                     depthImageView, colorImageView;
        std::vector<VkImageView>        swapchainImageViews;
        VkExtent2D                      swapchainExtent;
        VkFormat                        swapchainImageFormat;
        VkRenderPass                    *renderPass;
        
        void createFramebuffers();
        void createColorResources();
        void createDepthResources();
    
};
#endif