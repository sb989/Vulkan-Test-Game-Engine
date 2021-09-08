#ifndef __VTGE_SWAPCHAIN_HPP__
#define __VTGE_SWAPCHAIN_HPP
#define GLFW_INCLUDE_VULKAN
#include <vector>
#include <GLFW/glfw3.h>
#include "vtge_queuefamilyindices.hpp"

struct SwapchainSupportDetails;



class Swapchain{
    public:
        Swapchain(VkSurfaceKHR *surface, GLFWwindow *window);
        ~Swapchain();
        SwapchainSupportDetails querySwapchainSupport();
        VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& 
            availableFormats);
        VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);
        VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities);
        
        VkSwapchainKHR                  swapchain;
        std::vector<VkImage>            swapchainImages;
        std::vector<VkImageView>        swapchainImageViews;
        VkFormat                        swapchainImageFormat;
        VkExtent2D                      swapchainExtent;

    private:
        void createSwapchain();
        void createImageViews(); 
        
        VkSurfaceKHR                    *surface;
        GLFWwindow                      *window;
        

};

#endif