#ifndef __VTGE_OBJECT_HPP__
#define __VTGE_OBJECT_HPP__
#include <vector>
#include <string>
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include<glm/glm.hpp>

class Model;
class Swapchain;
class Pipeline;
struct Material{
    alignas(16) glm::vec3 ambient;
    alignas(16) glm::vec3 diffuse;
    alignas(16) glm::vec3 specular;
    alignas(4) float shininess;
};
class Object{
    public:
        Object(std::string modelPath, uint32_t imageCount, std::string diffuseMapPath = "", std::string specularMapPath = "", glm::vec4 color = {0,0,0,255});
        ~Object();
        static void destroyAllObjects();
        static void cleanupAllMemory();
        void drawObject(VkCommandBuffer *commandBuffer, VkPipelineLayout pipelineLayout, int index);
        static void drawAllObjects(VkCommandBuffer * commandBuffer, Pipeline *pipeline, int index);
        void updateObject(uint32_t currentImage, glm::mat4 projection, glm::mat4 view);
        static void updateAllObjects(uint32_t currentImage, glm::mat4 projection, glm::mat4 view);
        static void recreateAllObjects(uint32_t imageCount);
        bool getIsVisible(){return isVisible;}
        void setIsVisible(bool visible){isVisible = visible;}
        Model *getModel(){return m;}
        static VkDescriptorSetLayout * getDescriptorSetLayout();
        void recreateUBufferPoolSets(uint32_t imageCount);
        void setImageCount(uint32_t imageCount);
        static void destroyDescriptorSetLayout();
    private:
        Model *m;
        bool isVisible;
        void createDescriptorPool();
        void createDescriptorSets();
        void updateUniformBuffer(uint32_t currentImage, glm::mat4 projection, glm::mat4 view);
        void updateMaterial(Material mat);
        void cleanupMemory();
        void setupDescriptorSetLayout();
        void createDescriptorBuffers();
        std::vector<VkBuffer> material, uniformBuffers;
        std::vector<VkDeviceMemory> uniformBuffersMemory, materialMemory;
        std::vector<VkDescriptorSet> * descriptorSets;
        //Material mtrl;
        uint32_t imageCount;
        VkDescriptorPool *descriptorPool;
        static VkDescriptorSetLayout * descriptorSetLayout;


};
#endif