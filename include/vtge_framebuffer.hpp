#ifndef __VTGE_FRAMEBUFFER_HPP__
#define __VTGE_FRAMEBUFFER_HPP__
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <vector>
class Swapchain;

class Framebuffer
{
public:
    /**
         * @brief constructor for a framebuffer object
         * @param swapchain pointer to the swapchain object that the frambuffer is using
         * @param renderPass pointe to the render pass object that the framebuffer submits to
         */
    Framebuffer(Swapchain *swapchain, VkRenderPass *renderPass);

    /** 
         * @brief destructor for a framebuffer object; cleans up and destroys anything created by the framebuffer object 
         */
    ~Framebuffer();

    std::vector<VkFramebuffer> swapchainFramebuffers;

private:
    std::vector<VkImageView> swapchainImageViews;
    VkImage depthImage, colorImage;
    VkDeviceMemory depthImageMemory, colorImageMemory;
    VkImageView depthImageView, colorImageView;
    VkExtent2D swapchainExtent;
    VkFormat swapchainImageFormat;
    VkRenderPass *renderPass;

    /**
         * @brief creates the framebuffers for each swapchain image ciew
         */
    void createFramebuffers();

    /**
         * @brief creates color resources for the framebuffer
         */
    void createColorResources();

    /**
         * @brief creates the depth resources for the framebuffer
         */
    void createDepthResources();
};
#endif