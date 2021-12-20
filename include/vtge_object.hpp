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

class Object{
    public:
        Object(std::string modelPath, uint32_t imageCount, std::string diffuseMapPath = "", std::string specularMapPath = "", glm::vec4 color = {-1,-1,-1,-1});
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
        void setImageCount(uint32_t imageCount);
    private:
        Model *m;
        bool isVisible;
        void cleanupMemory();
        uint32_t imageCount;


};
#endif