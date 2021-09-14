#ifndef __VTGE_TEXTURE_HPP__
#define __VTGE_TEXTURE_HPP__
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <string>


class Texture{
    public:
        /**
         * @brief the constructor for a texture object
         * @param texturePath the path to the texture
         */
        Texture(std::string texturePath);

        /**
         * @brief the destructor for a texture object
         */
        ~Texture();
        VkImage         textureImage;
        VkImageView     textureImageView;
        VkDeviceMemory  textureImageMemory;
        VkSampler       textureSampler;
    private:
        std::string texturePath;
        int         texWidth;
        int         texHeight;
        int         texChannels;
        uint32_t    mipLevels;

        /**
         * @brief creates the VkImage and creates its mipmaps 
         */
        void createTextureImage();

        /**
         * @brief generates the mipmaps for a texture
         * @param image the VkImage that a mipmap is beign created for
         * @param commandBuffer the command buffer to use to store commands
         * @param texWidth the width of the texture
         * @param texHeight the height of the texture
         * @param mipLevels the number of miplevels
         */
        void generateMipmaps(VkImage image, VkFormat imageFormat, VkCommandBuffer commandBuffer, int32_t texWidth, int32_t texHeight, uint32_t mipLevels);

        /**
         * @brief creates an image view for the texture vkimage
         */
        void createTextureImageView();

        /**
         * @brief creates a texture sampler for the texture
         */
        void createTextureSampler();

        /**
         * @brief copies the image info from a buffer to the VkImage
         */
        void copyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height);

};

#endif