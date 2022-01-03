#ifndef __VTGE_SWAPCHAIN_HPP__
#define __VTGE_SWAPCHAIN_HPP
#define GLFW_INCLUDE_VULKAN
#include <vector>
#include <GLFW/glfw3.h>
#include "vtge_getter_and_checker_functions.hpp"
struct SwapchainSupportDetails;

class Swapchain
{
public:
    /**
         * @brief contructor for a swapchain object
         * @param surface pointer to the surface being used
         * @param window point to the window beign used
         * @param swapchainSupport the swapchain support details of the physical device
         */
    Swapchain(VkSurfaceKHR *surface, GLFWwindow *window, SwapchainSupportDetails swapchainSupport);

    /**
         * @brief the destructor for a swapchain object
         */
    ~Swapchain();

    /**
         * @brief picks a swapsurface format that meets crtieria from the list of availabe formats
         * @param availabeFormats a list of available formats to pick from
         * @return returns a VkSurfaceFormatKHR that matches the criteria or picks a default one that is always available
         */
    VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR> &
                                                   availableFormats);

    /**
         * @brief picks a swap present mode that meets criteria from the list of available present modes
         * @param availablePresentModes a list of available present modes to pick from
         * @return returns a VkPresentModeKHR that matches the criteria or picks a default one that is always available
         */
    VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR> &availablePresentModes);

    /**
         * @brief picks a swap extent based on the surface capabilites
         * @param capabilities the capabilites of the surface to decided the extent from
         * @return returns a VkExtent2D
         */
    VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR &capabilities);

    std::vector<VkImage> swapchainImages;
    std::vector<VkImageView> swapchainImageViews;
    VkSwapchainKHR swapchain;
    VkFormat swapchainImageFormat;
    VkExtent2D swapchainExtent;

private:
    SwapchainSupportDetails swapchainSupport;
    VkSurfaceKHR *surface;
    GLFWwindow *window;
    /**
         * @brief creates the VkSwapchainKHR object
         */
    void createSwapchain();

    /**
         * @brief creates an image view for each swapchain image 
         */
    void createImageViews();
};

#endif