#ifndef __VTGE_IMAGE_HPP__
#define __VTGE_IMAGE_HPP__
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
namespace image{
    /**
     * @brief creates a VkImage
     * @param width width of the image
     * @param height height of the image
     * @param mipLevels the number of mipLevels the image has
     * @param numSamples the number of samples the image can have
     * @param format the format the image should have
     * @param tiling the tiling the image should have
     * @param usage the usage flag
     * @param properties the memory properties the image should have
     * @param image a reference to the image being created
     * @param imageMemory a reference to the memory being allocated
     */
    void createImage(uint32_t width, uint32_t height, uint32_t mipLevels, VkSampleCountFlagBits numSamples, 
        VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, 
        VkImage& image, VkDeviceMemory& imageMemory);

    /**
     * @brief determines if the format has a stencil component
     * @param format the format being checked for a stencil component
     * @return returns true if it the format has a stencil format and false otherwise
     */
    bool hasStencilComponent(VkFormat format);

    /**
     * @brief transitions an image from its current layout to the new layout
     * @param image the image being transitioned
     * @param format the current format of the image
     * @param oldLayout the current layout of the image
     * @param newLayout the new layout of the image
     * @param commandBuffer the command buffer being used to store the transition command
     * @param mipLevels the number of mipLevels the image has
     */
    void transitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout, 
    VkCommandBuffer commandBuffer, uint32_t mipLevels);

    /**
     * @brief create an image view for an image
     * @param image the image the image view is being created for
     * @param format the format of the image
     * @param aspectFlags a flag that determines the image aspect
     * @param mipLevels thhe number of mipLevels the image has
     * @return returns the VkImageView created
     */
    VkImageView createImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags, uint32_t mipLevels);
        
}

#endif