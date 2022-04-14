#ifndef __VTGE_LIGHT_HPP__
#define __VTGE_LIGHT_HPP__
#define GLFW_INCLUDE_VULKAN
#include <string>
#include <vector>
#include <glm/glm.hpp>
#include <GLFW/glfw3.h>
#include "vtge_ubo.hpp"

const static int MAX_LIGHT_COUNT = 1000;
struct DirectionalLightInfo
{
    // alignas(4) int numLights;
    alignas(16) glm::vec4 diffuse;
    alignas(16) glm::vec4 specular;
    alignas(16) glm::vec4 ambient;
    alignas(16) glm::vec4 direction;
    alignas(16) glm::mat4 lightView;
    alignas(16) glm::mat4 lightProjection;
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
    alignas(16) glm::mat4 lightView;
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

enum LightType
{
    Point,
    Direction,
    Spot
};

class Model;
class Pipeline;
class Swapchain;
class DepthBuffer;
class Light
{
public:
    Light(
        std::string modelPath, glm::vec4 lightPos, uint32_t imageCount,
        glm::vec4 diffuse, glm::vec4 ambient, glm::vec4 specular,
        float constant, float linear, float quadratic, std::string colorName);
    Light(
        std::string modelPath, glm::vec4 lightPos, glm::vec4 direction, uint32_t imageCount,
        glm::vec4 diffuse, glm::vec4 ambient, glm::vec4 specular, std::string colorName);
    Light(
        std::string modelPath, glm::vec4 lightPos, glm::vec4 direction, uint32_t imageCount,
        glm::vec4 diffuse, glm::vec4 ambient, glm::vec4 specular,
        float constant, float linear, float quadratic, float cutOff, float outerCutOff, std::string colorName);

    static void createPointLight(
        std::string modelPath, glm::vec3 scale, glm::vec3 rotate, glm::vec4 lightPos,
        glm::vec4 diffuse, glm::vec4 ambient, glm::vec4 specular, float constant, float linear,
        float quadratic, std::string color);

    static void createDirectionalLight(
        std::string modelPath, glm::vec3 scale, glm::vec3 rotate, glm::vec3 direction, glm::vec4 lightPos,
        glm::vec4 diffuse, glm::vec4 ambient, glm::vec4 specular, std::string color);

    static void createSpotLight(
        std::string modelPath, glm::vec3 scale, glm::vec3 rotate, glm::vec3 direction, glm::vec4 lightPos,
        glm::vec4 diffuse, glm::vec4 ambient, glm::vec4 specular, float constant, float linear, float quadratic,
        float cutOff, float outerCutOff, std::string color);
    ~Light();
    static void initLights(int imageCount = -1);
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
    static void drawShadows(uint32_t index);
    void drawShadow(uint32_t index, uint32_t lightNum);
    Model *getModel() { return m; }
    static std::vector<VkBuffer> *getDirectionalLightBuffers();
    static std::vector<VkDeviceMemory> *getDirectionalLightBufferMemory();
    static VkDescriptorSetLayout *getDescriptorSetLayout();
    static std::vector<VkDescriptorSet> *getDescriptorSets();
    static VkDescriptorSetLayout *getShadowMapDescriptorSetLayout();
    static std::vector<VkDescriptorSet> *getShadowMapDescriptorSet();
    static VkDeviceSize getDirectionalLightBufferSize();
    static Light *getDirectionalLight(uint32_t i);
    static Light *getPointLight(uint32_t i);
    static Light *getSpotLight(uint32_t i);
    static void setImageCount(uint32_t);
    static void populateAndDrawShadows(uint32_t index);
    static void populateShadowDepthCmdBuffers(uint32_t index);
    static void populateAllShadowDepthCmdBuffers(uint32_t imageCount);
    static DepthBuffer *getDepthBuffer();
    glm::vec4 getDirection();
    glm::vec4 getPos();
    glm::mat4 getLightView();
    glm::mat4 getLightProjection();

private:
    Model *m;
    LightType type;
    static uint32_t imageCount;
    float constant, linear, quadratic, cutOff, outerCutOff;
    glm::vec4 lightPos, direction;
    glm::vec4 diffuse, ambient, specular;
    glm::mat4 lightView, lightProjection;
    UniformBufferObject ubo;
    static VkDescriptorSetLayout *descriptorSetLayout, *shadowMapDescriptorSetLayout;
    static std::vector<VkBuffer> directionalLightBuffers, pointLightBuffers, spotLightBuffers;
    static std::vector<VkDeviceMemory> directionalLightBuffersMemory, pointLightBuffersMemory, spotLightBuffersMemory;
    static VkDescriptorPool *descriptorPool;
    static std::vector<VkDescriptorSet> *descriptorSets;
    static std::vector<VkDescriptorSet> *shadowMapDescriptorSet;
    static DepthBuffer *directionalShadowDepthBuffer;
    void calculateLightViewMatrix();

    static void createDescriptorPool();
    static void createDescriptorSets();
    static void createShadowMapDescriptorSet();
    static void recreateUBufferPoolSets();
    static void setupDescriptorSetLayout();
    static void setupShadowMapDescriptorSetLayouts();
    void updateUniformBuffer(uint32_t currentImage, glm::mat4 projection, glm::mat4 view);
    static void updateLightBuffers(uint32_t currentImage, glm::mat4 projection, glm::mat4 view);
    static void updateDirectionalLightBuffer(uint32_t currentImage, glm::mat4 projection, glm::mat4 view,
                                             std::vector<Light *> lightList, std::vector<VkDeviceMemory> lightBuffersMemory);
    static void updateSpotLightBuffer(uint32_t currentImage, glm::mat4 projection, glm::mat4 view,
                                      std::vector<Light *> lightList, std::vector<VkDeviceMemory> lightBuffersMemory);
    static void updatePointLightBuffer(uint32_t currentImage, glm::mat4 projection, glm::mat4 view,
                                       std::vector<Light *> lightList, std::vector<VkDeviceMemory> lightBuffersMemory);
};

#endif