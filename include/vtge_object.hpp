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
    struct Materials{
        
    };
    
    class Object{
        public:
            Object(std::string modelPath, std::string texturePath, Swapchain * swapchain);
            Object(std::string modelPath, Swapchain * swapchain);
            ~Object();
            static void destroyAllObjects();
            static void cleanupAllModelMemory();
            void drawObject(VkCommandBuffer *commandBuffer, VkPipelineLayout pipelineLayout, int count);
            static void drawAllObjects(VkCommandBuffer * commandBuffer, Pipeline *pipeline, int count);
            void updateObject(uint32_t currentImage, glm::mat4 projection, glm::mat4 view);
            static void updateAllObjects(uint32_t currentImage, glm::mat4 projection, glm::mat4 view);
            void recreateModel(Swapchain *swapchain);
            static void recreateAllObjectsModel(Swapchain *swapchain);
            bool getIsVisible(){return isVisible;}
            void setIsVisible(bool visible){isVisible = visible;}
            Model *getModel(){return m;}
        private:
            Model *m;
            bool isVisible;

    };
#endif