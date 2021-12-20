#ifndef __VTGE_MODEL_HPP__
#define __VTGE_MODEL_HPP__
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include "vtge_buffer_helper_functions.hpp"
#include <vector>
#include "vtge_vertex.hpp"
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/hash.hpp>

namespace std {
    template<> struct hash<Vertex> {
        size_t operator()(Vertex const &vertex) const {
            return ((hash<glm::vec3>()(vertex.pos) ^
            (hash<glm::vec3> () (vertex.color) << 1)) >> 1) ^
            (hash<glm::vec2>()(vertex.texCoord) << 1);
        }
    };
}

struct Material{
    alignas(4) float shininess;
};

class Texture;
class Model{
    public:
       
        Model(std::string modelPath, uint32_t imageCount, std::string diffuseMapPath = "", std::string specularMapPath = "", glm::vec4 color = {-1,-1,-1,-1});
        ~Model();
        void recreateUBufferPoolSets(uint32_t imageCount);
        glm::mat4 getModelMat(){return modelMat;}
        glm::vec4 getModelPos();

        static VkDescriptorSetLayout * getDescriptorSetLayout();
        std::vector<VkDescriptorSet> * getDescriptorSets();


     
        void updateModelMat(uint32_t currentImage, glm::mat4 projection, glm::mat4 view);

        void setVelocity(glm::vec3 vel){velocity = vel;}

        void setRotation(glm::vec3 rot){rotation = rot;}

        void moveModel(glm::vec3 changeInPos);

        void scaleModel(glm::vec3 factor);
        void rotateModel(glm::vec3 rotation);
        void cleanupMemory();
        static void destroyDescriptorSetLayout();
        void recreateAllModels(uint32_t imageCount);        
        void updateMaterial(Material mat);
        std::vector<Vertex>             vertices;
        std::vector<uint32_t>           vertexIndices;
        //std::vector<VkDescriptorSet>    *descriptorSets;
        VkBuffer                        vertexBuffer, indexBuffer;
        VkDeviceMemory                  vertexBufferMemory, indexBufferMemory;
        //VkDescriptorPool                *descriptorPool;
        Texture                         *diffuseMap, *specularMap;
    private:
        uint32_t imageCount;
        static VkDescriptorSetLayout *descriptorSetLayout;
        std::string                     modelPath, diffuseMapPath, specularMapPath;
        glm::mat4                       modelMat;
        glm::vec3                       velocity;
        glm::vec3                       rotation;
        std::vector<VkDeviceMemory> uniformBuffersMemory, materialMemory;
        std::vector<VkBuffer> uniformBuffers, material;
        VkDescriptorPool *descriptorPool;
        std::vector<VkDescriptorSet>    *descriptorSets;
      
        void createDescriptorBuffers();
        
        void createDescriptorPool();

        void createDescriptorSets();

        void createVertexBuffer();

        void createIndexBuffer();

        void createBufferAndCopy(VkDeviceSize bufferSize, VkBuffer *buffer, VkDeviceMemory *deviceMemory, VkBufferUsageFlags flags,void *pointer);

        void loadModel();

        void createDescriptorSetLayout();

        void setupDescriptorSetLayout();
        
        void initDescriptorSets(uint32_t imageCount);
};

#endif