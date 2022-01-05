#ifndef __VTGE_TEXTURE_HPP__
#define __VTGE_TEXTURE_HPP__
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <string>
#include <unordered_map>
#include <glm/glm.hpp>

class Texture
{
public:
     Texture(std::string texturePath);
     Texture(int height, int width, glm::vec4 color);

     ~Texture();

     static Texture *createTextureFromImage(std::string texturePath);
     static Texture *createTextureFromColor(std::string colorName, int height, int width, glm::vec4 color);
     static void destroyAllTextures();

     VkImage textureImage;
     VkImageView textureImageView;
     VkDeviceMemory textureImageMemory;
     VkSampler textureSampler;

private:
     std::string texturePath;
     int texWidth;
     int texHeight;
     int texChannels;
     uint32_t mipLevels;
     static std::unordered_map<std::string, Texture *> textureMap;

     void createTextureImage();

     void generateMipmaps(VkImage image, VkFormat imageFormat, VkCommandBuffer commandBuffer, int32_t texWidth, int32_t texHeight, uint32_t mipLevels);

     void createTextureImageView();

     void createTextureSampler();

     void copyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height);

     void createSolidColorTexture(int width, int height, glm::vec4 color);
};

#endif