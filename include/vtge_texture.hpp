#ifndef __VTGE_TEXTURE_HPP__
#define __VTGE_TEXTURE_HPP__
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <string>


class Texture{
    public:
        VkImage textureImage;
        VkImageView textureImageView;
        VkDeviceMemory textureImageMemory;
        VkSampler textureSampler;
        Texture(std::string texturePath);
        ~Texture();
    private:
        std::string texturePath;
        int texWidth;
        int texHeight;
        int texChannels;
        uint32_t mipLevels;
       
        //VkQueue *transferQueue, *graphicsQueue;
        //VkCommandPool *transferCommandPool, *graphicsCommandPool;
        //VkBuffer stagingBuffer;
        void createTextureImage();
        void generateMipmaps(VkImage image, VkFormat imageFormat, VkCommandBuffer commandBuffer, int32_t texWidth, int32_t texHeight, uint32_t mipLevels);
        void createTextureImageView();
        void createTextureSampler();
        void copyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height);

};

#endif