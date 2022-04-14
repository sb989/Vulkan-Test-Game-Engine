#ifndef __VTGE_OBJECT_HPP__
#define __VTGE_OBJECT_HPP__
#include <vector>
#include <string>
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

class Model;
class Swapchain;
class Pipeline;

class Object
{
public:
    Object(std::string modelPath, uint32_t imageCount, std::string diffuseMapPath = "", std::string specularMapPath = "", glm::vec4 color = {-1, -1, -1, -1}, std::string colorName = "");
    ~Object();
    static void destroyAllObjects();
    static void cleanupAllMemory();
    void drawObject(VkCommandBuffer *commandBuffer, VkPipelineLayout pipelineLayout, int index);
    static void drawAllObjects(VkCommandBuffer *commandBuffer, Pipeline *pipeline, int index);
    void updateObject(uint32_t currentImage, glm::mat4 projection, glm::mat4 view);
    static void updateAllObjects(uint32_t currentImage, glm::mat4 projection, glm::mat4 view);
    static void recreateAllObjects(uint32_t imageCount);
    bool getIsVisible() { return isVisible; }
    void setIsVisible(bool visible) { isVisible = visible; }
    Model *getModel();
    void setImageCount(uint32_t imageCount);
    static void createObject(std::string modelPath, glm::vec3 translate, glm::vec3 scale, glm::vec3 rotate, std::string diffuseMapPath = "", std::string specularMapPath = "");
    static void createObject(std::string modelPath, glm::vec3 translate, glm::vec3 scale, glm::vec3 rotate, glm::vec4 color, std::string colorName);

    static void setObjectTransform(Object *obj, glm::vec3 translate, glm::vec3 scale, glm::vec3 rotate);
    static std::vector<Object *> getObjectList();

    int objIndex;

private:
    Model *m;
    bool isVisible;
    void cleanupMemory();
    uint32_t imageCount;
};
#endif