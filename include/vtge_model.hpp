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
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

struct Node;
class Mesh;
class Texture;

namespace std
{
    template <>
    struct hash<Vertex>
    {
        size_t operator()(Vertex const &vertex) const
        {
            return (hash<glm::vec3>()(vertex.pos) >> 1) ^
                   (hash<glm::vec2>()(vertex.texCoord) << 1);
        }
    };
}

class Model
{
public:
    Model(std::string modelPath, uint32_t imageCount, std::string diffuseMapPath = "", std::string specularMapPath = "", glm::vec4 color = {-1, -1, -1, -1}, std::string colorName = "");

    ~Model();
    glm::mat4 getModelMat();
    glm::vec4 getModelPos();
    std::string getModelPath();

    void updateModelMat(uint32_t currentImage, glm::mat4 projection, glm::mat4 view);

    void setVelocity(glm::vec3 vel) { velocity = vel; }

    void setRotation(glm::vec3 rot) { rotation = rot; }
    std::vector<Mesh *> getMeshList();
    void moveModel(glm::vec3 changeInPos);
    void scaleModel(glm::vec3 factor);
    void rotateModel(glm::vec3 rotation);
    void cleanupMemory();
    void recreateModel(uint32_t imageCount);
    void drawModel(std::vector<VkDescriptorSet> combinedDescriptorSets, VkCommandBuffer *commandBuffer, VkPipelineLayout pipelineLayout, int index);

private:
    std::string modelPath, diffuseMapPath, specularMapPath, directory, colorName;
    glm::mat4 modelMat;
    glm::vec3 velocity;
    glm::vec3 rotation;
    glm::vec4 color;
    std::vector<Mesh *> meshes;
    uint32_t imageCount;
    void processNode(aiNode *node, const aiScene *scene, Node *parentNode);
    Mesh *processMesh(aiMesh *mesh, const aiScene *scene, Node *node);
    std::vector<Texture *> loadMaterialTextures(aiMaterial *mat, aiTextureType type);
    std::vector<Node *> nodeList;
    void loadModel();
    void assimpLoadModel();
};

#endif