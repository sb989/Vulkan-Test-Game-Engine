#ifndef __VTGE_LIGHT_HPP__
#define __VTGE_LIGHT_HPP__
#include <string>
#include <vector>
#include<glm/glm.hpp>
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
class Model;
class Swapchain;
class Pipeline;
class Light{
    public:
        Light(std::string modelPath, Swapchain *swapchain, glm::vec3 lightColor, glm::vec3 lightPos);
        ~Light();
        static void destroyAllLights();
        void recreateUBufferPoolSets(Swapchain *swapchain);
        static void recreateAllLights(Swapchain *swapchain);
        void cleanupMemory(Swapchain *swapchain);
        static void cleanupAllMemory(Swapchain *swapchain);
        void updateUniformBuffer(uint32_t currentImage);
        void updateLight(uint32_t currentImage, glm::mat4 projection, glm::mat4 view);
        static void updateAllLights(uint32_t currentImage, glm::mat4 projection, glm::mat4 view);
        Model *getModel(){return m;}
        void drawLight(VkCommandBuffer *commandBuffer, VkPipelineLayout pipelineLayout, int count);
        static void drawAllLights(VkCommandBuffer *commandBuffer, Pipeline *pipeline, int count);
    private:
        Model *m;
        glm::vec3 lightColor, lightPos;
        static VkDescriptorSetLayout *descriptorSetLayout;
        VkDescriptorPool *descriptorPool;
        std::vector<VkDescriptorSet>    *descriptorSets;
        void createDescriptorPool(Swapchain *swapchain);
        void createDescriptorSets(Swapchain *swapchain);
        void setupDescriptorSetLayout();
};

#endif