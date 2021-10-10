#ifndef __VTGE_LIGHT_HPP__
#define __VTGE_LIGHT_HPP__
#include <string>
#include <vector>
#include<glm/glm.hpp>
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include "vtge_ubo.hpp"
class Model;
class Pipeline;
struct LightInfo{
    alignas(16) glm::vec3 lightcolor;
    alignas(16) glm::vec3 lightpos;
    alignas(16) UniformBufferObject ubo;
};



class Light{
    public:
        Light(std::string modelPath, glm::vec3 lightColor, glm::vec3 lightPos, uint32_t imageCount);
        ~Light();
        static void destroyAllLights();
        static void recreateAllLights(uint32_t imageCount);
        static void cleanupMemory();
        static void cleanupAllMemory();
        void updateUniformBuffer(uint32_t currentImage);
        void updateLight(uint32_t currentImage, glm::mat4 projection, glm::mat4 view);
        static void updateAllLights(uint32_t currentImage, glm::mat4 projection, glm::mat4 view);
        Model *getModel(){return m;}
        void drawLight(VkCommandBuffer *commandBuffer, VkPipelineLayout pipelineLayout, int index, int instance);
        static void drawAllLights(VkCommandBuffer *commandBuffer, Pipeline *pipeline, int index);
        static void destroyDescriptorSetLayout();
        void recreateLightBuffer();
        static std::vector<VkBuffer> * getLightBuffers();
        static std::vector<VkDeviceMemory> * getLightBufferMemory();
        static VkDescriptorSetLayout * getDescriptorSetLayout();
        static VkDeviceSize getLightBufferSize();
        static void destroyLightBufferAndMemory(size_t imageCount);
        static Light * getLight(uint32_t i);
    private:
        Model *m;
        uint32_t imageCount;
        glm::vec3 lightColor, lightPos;
        UniformBufferObject ubo;
        //std::vector<VkBuffer> uniformBuffers;
        //std::vector<VkDeviceMemory> uniformBuffersMemory;
        void createDescriptorPool();
        void createDescriptorSets();
        //void createDescriptorBuffers();
        void recreateUBufferPoolSets();
        void setupDescriptorSetLayout();
        void updateUniformBuffer(uint32_t currentImage, glm::mat4 projection, glm::mat4 view);
        static void updateLightBuffer(uint32_t currentImage, glm::mat4 projection, glm::mat4 view);


        static VkDescriptorSetLayout *descriptorSetLayout;
        const static int MAX_LIGHT_COUNT = 10000;
        static std::vector<VkBuffer>           lightBuffers;
        static std::vector<VkDeviceMemory>     lightBuffersMemory;
        static VkDescriptorPool *descriptorPool;
        static std::vector<VkDescriptorSet>    *descriptorSets;
};

#endif