#ifndef __VTGE_SWAPCHAIN_HPP__
#define __VTGE_SWAPCHAIN_HPP__
#define GLFW_INCLUDE_VULKAN
#include <vector>
#include <GLFW/glfw3.h>
struct SwapchainSupportDetails;

class Swapchain
{
public:
     Swapchain(VkSurfaceKHR *surface, GLFWwindow *window, SwapchainSupportDetails *swapchainSupport);

     ~Swapchain();

     VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR> &
                                                    availableFormats);

     VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR> &availablePresentModes);

     VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR &capabilities);

     std::vector<VkImage> swapchainImages;
     std::vector<VkImageView> swapchainImageViews;
     VkSwapchainKHR swapchain;
     VkFormat swapchainImageFormat;
     VkExtent2D swapchainExtent;

private:
     SwapchainSupportDetails *swapchainSupport;
     VkSurfaceKHR *surface;
     GLFWwindow *window;

     void createSwapchain();

     void createImageViews();
};

#endif