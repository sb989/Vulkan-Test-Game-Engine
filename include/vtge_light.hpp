#ifndef __VTGE_LIGHT_HPP__
#define __VTGE_LIGHT_HPP__
#include <string>
#include <vector>
#include <glm/glm.hpp>
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include "vtge_ubo.hpp"

const static int MAX_LIGHT_COUNT = 10000;
struct DirectionalLightInfo
{
    // alignas(4) int numLights;
    alignas(16) glm::vec4 diffuse;
    alignas(16) glm::vec4 specular;
    alignas(16) glm::vec4 ambient;
    alignas(16) glm::vec4 direction;
};

struct SpotLightInfo
{
    // alignas(4) int numLights;
    alignas(4) float constant;
    alignas(4) float linear;
    alignas(4) float quadratic;
    alignas(4) float cutOff;
    alignas(4) float outerCutOff;
    alignas(16) glm::vec4 direction;
    alignas(16) glm::vec4 lightpos;
    alignas(16) glm::vec4 diffuse;
    alignas(16) glm::vec4 specular;
    alignas(16) glm::vec4 ambient;
};

struct PointLightInfo
{
    // alignas(4) int numLights;
    alignas(4) float constant;
    alignas(4) float linear;
    alignas(4) float quadratic;
    alignas(16) glm::vec4 lightpos;
    alignas(16) glm::vec4 diffuse;
    alignas(16) glm::vec4 specular;
    alignas(16) glm::vec4 ambient;
};
class Model;
class Pipeline;
class Swapchain;
class Light
{
public:
    Light(std::string modelPath, glm::vec4 lightPos, uint32_t imageCount,
          glm::vec4 diffuse, glm::vec4 ambient, glm::vec4 specular,
          float constant, float linear, float quadratic, std::string colorName);
    Light(std::string modelPath, glm::vec4 lightPos, glm::vec4 direction, uint32_t imageCount,
          glm::vec4 diffuse, glm::vec4 ambient, glm::vec4 specular, std::string colorName);
    Light(std::string modelPath, glm::vec4 lightPos, glm::vec4 direction, uint32_t imageCount,
          glm::vec4 diffuse, glm::vec4 ambient, glm::vec4 specular,
          float constant, float linear, float quadratic, float cutOff, float outerCutOff, std::string colorName);

    static void createPointLight(std::string modelPath, glm::vec3 scale, glm::vec3 rotate, glm::vec4 lightPos,
                                 glm::vec4 diffuse, glm::vec4 ambient, glm::vec4 specular, float constant, float linear,
                                 float quadratic, std::string color);

    static void createDirectionalLight(std::string modelPath, glm::vec3 scale, glm::vec3 rotate, glm::vec3 direction, glm::vec4 lightPos,
                                       glm::vec4 diffuse, glm::vec4 ambient, glm::vec4 specular, std::string color);

    static void createSpotLight(std::string modelPath, glm::vec3 scale, glm::vec3 rotate, glm::vec3 direction, glm::vec4 lightPos,
                                glm::vec4 diffuse, glm::vec4 ambient, glm::vec4 specular, float constant, float linear, float quadratic,
                                float cutOff, float outerCutOff, std::string color);
    ~Light();
    static void initLights();
    static void destroyAllLights();
    static void destroyDescriptorSetLayout();
    static void destroyLightBufferAndMemory(size_t imageCount, std::vector<VkBuffer> buffer, std::vector<VkDeviceMemory> memory);
    static void recreateAllLights(uint32_t imageCount);
    static void recreateLightBuffer();
    static void cleanupMemory();
    static void cleanupAllMemory();
    void updateUniformBuffer(uint32_t currentImage);
    void updateLight(uint32_t currentImage, glm::mat4 projection, glm::mat4 view);
    static void updateAllLights(uint32_t currentImage, glm::mat4 projection, glm::mat4 view);
    static void drawAllLights(VkCommandBuffer *commandBuffer, Pipeline *pipeline, int index);
    void drawLight(VkCommandBuffer *commandBuffer, VkPipelineLayout pipelineLayout, int index, int instance);
    Model *getModel() { return m; }
    static std::vector<VkBuffer> *getDirectionalLightBuffers();
    static std::vector<VkDeviceMemory> *getDirectionalLightBufferMemory();
    static VkDescriptorSetLayout *getDescriptorSetLayout();
    static std::vector<VkDescriptorSet> *getDescriptorSets();
    static VkDeviceSize getDirectionalLightBufferSize();
    static Light *getDirectionalLight(uint32_t i);
    static Light *getPointLight(uint32_t i);
    static Light *getSpotLight(uint32_t i);
    static void setImageCount(uint32_t);

private:
    Model *m;
    static uint32_t imageCount;
    float constant, linear, quadratic, cutOff, outerCutOff;
    glm::vec4 lightPos, direction;
    glm::vec4 diffuse, ambient, specular;
    UniformBufferObject ubo;
    static VkDescriptorSetLayout *descriptorSetLayout;
    static std::vector<VkBuffer> directionalLightBuffers, pointLightBuffers, spotLightBuffers;
    static std::vector<VkDeviceMemory> directionalLightBuffersMemory, pointLightBuffersMemory, spotLightBuffersMemory;
    static VkDescriptorPool *descriptorPool;
    static std::vector<VkDescriptorSet> *descriptorSets;
    //std::vector<VkBuffer> uniformBuffers;
    //std::vector<VkDeviceMemory> uniformBuffersMemory;
    static void createDescriptorPool();
    static void createDescriptorSets();
    //void createDescriptorBuffers();
    static void recreateUBufferPoolSets();
    static void setupDescriptorSetLayout();
    void updateUniformBuffer(uint32_t currentImage, glm::mat4 projection, glm::mat4 view);
    static void updateLightBuffers(uint32_t currentImage, glm::mat4 projection, glm::mat4 view);
    //template <typename LightInfoStruct>
    // static void updateLightBuffer(uint32_t currentImage, glm::mat4 projection, glm::mat4 view,
    //     std::vector<Light *> lightList, std::vector<VkDeviceMemory> lightBuffersMemory);
    static void updateDirectionalLightBuffer(uint32_t currentImage, glm::mat4 projection, glm::mat4 view,
                                             std::vector<Light *> lightList, std::vector<VkDeviceMemory> lightBuffersMemory);
    static void updateSpotLightBuffer(uint32_t currentImage, glm::mat4 projection, glm::mat4 view,
                                      std::vector<Light *> lightList, std::vector<VkDeviceMemory> lightBuffersMemory);
    static void updatePointLightBuffer(uint32_t currentImage, glm::mat4 projection, glm::mat4 view,
                                       std::vector<Light *> lightList, std::vector<VkDeviceMemory> lightBuffersMemory);
};

#endif