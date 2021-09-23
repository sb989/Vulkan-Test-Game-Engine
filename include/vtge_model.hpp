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
class Texture;
class Swapchain;

struct UniformBufferObject {
    alignas(16) glm::mat4 normMatrix;
    alignas(16) glm::mat4 modelView;
    alignas(16) glm::mat4 view;
    alignas(16) glm::mat4 proj;
};

struct LightInfo{
    alignas(16) glm::vec3 lightcolor;
    alignas(16) glm::vec3 lightpos;
};
const static int MAX_LIGHT_COUNT = 10000;
static std::vector<VkBuffer>           lightBuffers;
static std::vector<VkDeviceMemory>     lightBuffersMemory;
static VkDescriptorSetLayout *descriptorSetLayout;
class Model{
    public:
        /**
         * @brief the constructor for a model object
         * @param modelPath the path to the model
         * @param texturePath the path to the models texture
         * @param swapchain a pointer to the swapchain being used
         */
        Model(std::string modelPath, std::string texturePath, Swapchain *swapchain);
        Model(std::string modelPath, Swapchain *Swapchain);
       
        /**
         * @brief the destructor for a model object
         */
        ~Model();

        /**
         * @brief recreates the uniform buffer, descriptor pool and descriptor sets for a model
         * @param swapchain the swapchain being used
         */
        void recreateUBufferPoolSets(Swapchain *swapchain);

        /**
         * @brief returns the model matrix
         * @return returns a glm::mat4 that contains the model matrix
         */
        glm::mat4 getModelMat(){return modelMat;}

        /**
         * @brief returns the models position
         * @return returns a glm::vec3 decribing the models position
         */
        glm::vec3 getModelPos(){return glm::vec3(modelMat[3][0],modelMat[3][1], modelMat[3][2]);}

        /**
         * @brief updates the model matrix using the models velocity and rate of rotation
         */
        void updateModelMat();

        /**
         * @brief sets the models velocity
         * @param vel the new velocity of the model
         */
        void setVelocity(glm::vec3 vel){velocity = vel;}

        /**
         * @brief sets the models rotation rate
         * @param rot the new rate of rotation of the model
         */
        void setRotation(glm::vec3 rot){rotation = rot;}

        /**
         * @brief moves the model a distance in a direction
         * @param changeInPos a vec3 denoting the distance and direction
         */
        void moveModel(glm::vec3 changeInPos);

        /**
         * @brief scales the model by a factor
         * @param factor a vec3 denoting the factor to scale by
         */
        void scaleModel(glm::vec3 factor);

        /**
         * @brief rotates the model by the angle determined by the vector
         * @param a vec3 denoting the angle to rotate by
         */
        void rotateModel(glm::vec3 rotation);

        void setSwapchain(Swapchain *swapchain);

        void cleanupMemory();
        void updateUniformBuffer(uint32_t currentImage, glm::mat4 projection, glm::mat4 view);
        void recreateLightBuffer();
        static void destroyDescriptorSetLayout();
        static VkDescriptorSetLayout * getDescriptorSetLayout();
        static std::vector<VkBuffer> * getLightBuffers();
        static std::vector<VkDeviceMemory> *getLightBufferMemory();
        static void destroyLightBufferAndMemory(size_t imageCount);
        std::vector<VkBuffer>           uniformBuffers;
        std::vector<VkDeviceMemory>     uniformBuffersMemory;
        std::vector<Vertex>             vertices;
        std::vector<uint32_t>           vertexIndices;
        std::vector<VkDescriptorSet>    *descriptorSets;
        VkBuffer                        vertexBuffer, indexBuffer;
        VkDeviceMemory                  vertexBufferMemory, indexBufferMemory;
        VkDescriptorPool                *descriptorPool;
    private:
        std::string                     modelPath, texturePath;
        Texture                         *texture;
        Swapchain                       *swapchain;
        glm::mat4                       modelMat;
        glm::vec3                       velocity;
        glm::vec3                       rotation;

        /**
         * @brief creates a uniform buffer for each swapchain image
         */
        void createUniformBuffers();

       void createDescriptorBuffer(VkDeviceSize bufferSize, std::vector<VkBuffer> *buffers, std::vector<VkDeviceMemory> *bufferMemory, VkBufferUsageFlags bufferUsage);

        /**
         * @brief creates a descriptor pool 
         */
        void createDescriptorPool();

        /**
         * @brief allocates a descriptor set for each swapchain image from the descriptor pool
         */
        void createDescriptorSets();

        /**
         * @brief creates the vertex buffer from the info in the file
         */
        void createVertexBuffer();

        /**
         * @brief creates the index buffer from the info in the file
         */
        void createIndexBuffer();

        void createBufferAndCopy(VkDeviceSize bufferSize, VkBuffer *buffer, VkDeviceMemory *deviceMemory, VkBufferUsageFlags flags,void *pointer);

        /**
         * @brief loads the model info from the file
         */
        void loadModel();

        void createDescriptorSetLayout();

        void setupDescriptorSetLayout();
        
};

#endif