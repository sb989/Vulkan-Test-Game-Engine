#include "vtge_model.hpp"
#include "vtge_texture.hpp"
#include "vtge_descriptor.hpp"
#include "vtge_ubo.hpp"
#include "vtge_mesh.hpp"
#include "glm/gtx/string_cast.hpp"
#include <regex>
#include <cstring>
#include <unordered_map>
#include <vector>
#include <array>
#include <iostream>
Model::Model(std::string modelPath, uint32_t imageCount, std::string diffuseMapPath, std::string specularMapPath, glm::vec4 color, std::string colorName)
{
    this->modelPath = modelPath;
    this->diffuseMapPath = diffuseMapPath;
    this->specularMapPath = specularMapPath;
    this->color = color;
    this->colorName = colorName;
    this->imageCount = imageCount;
    assimpLoadModel();
    this->modelMat = glm::mat4(1.0f);
    this->velocity = glm::vec3(0.0f);
    this->rotation = glm::vec3(0.0f);
}

Model::~Model()
{
    for (int i = 0; i < meshes.size(); i++)
    {
        delete meshes[i];
    }
}

void Model::recreateModel(uint32_t imageCount)
{
    for (int i = 0; i < meshes.size(); i++)
    {
        meshes[i]->recreateUBufferPoolSets(imageCount);
    }
}

void Model::assimpLoadModel()
{
    Assimp::Importer import;
    const aiScene *scene = import.ReadFile(modelPath, aiProcess_Triangulate | aiProcess_FlipUVs);

    if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
    {
        std::cout << "ERROR::ASSIMP::" << import.GetErrorString() << std::endl;
        return;
    }
    directory = modelPath.substr(0, modelPath.find_last_of('/'));
    glm::mat4 id = glm::mat4(1.0f);
    processNode(scene->mRootNode, scene, nullptr);
}

void Model::processNode(aiNode *node, const aiScene *scene, Node *parentNode)
{
    //following learnopengl tutorial
    //https://learnopengl.com/Model-Loading/Model
    Node *n = new Node();
    n->transform = node->mTransformation;
    n->parent = parentNode;
    for (unsigned int i = 0; i < node->mNumMeshes; i++)
    {
        aiMesh *mesh = scene->mMeshes[node->mMeshes[i]];
        meshes.push_back(processMesh(mesh, scene, n));
    }
    // then do the same for each of its children
    for (unsigned int i = 0; i < node->mNumChildren; i++)
    {
        processNode(node->mChildren[i], scene, n);
    }
}

Mesh *Model::processMesh(aiMesh *mesh, const aiScene *scene, Node *node)
{
    //following learnopengl tutorial
    //https://learnopengl.com/Model-Loading/Model
    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;
    std::vector<Texture *> diffuseMaps;
    std::vector<Texture *> specularMaps;

    for (unsigned int i = 0; i < mesh->mNumVertices; i++)
    {
        Vertex vertex;
        // process vertex positions, normals and texture coordinates
        glm::vec3 vector;
        vector.x = mesh->mVertices[i].x;
        vector.y = mesh->mVertices[i].y;
        vector.z = mesh->mVertices[i].z;
        vertex.pos = vector;

        vector.x = mesh->mNormals[i].x;
        vector.y = mesh->mNormals[i].y;
        vector.z = mesh->mNormals[i].z;
        vertex.normal = vector;

        if (mesh->mTextureCoords[0])
        { // does the mesh contain texture coordinates?
            glm::vec2 vec;
            vec.x = mesh->mTextureCoords[0][i].x;
            vec.y = mesh->mTextureCoords[0][i].y;
            vertex.texCoord = vec;
        }
        else
            vertex.texCoord = glm::vec2(0.0f, 0.0f);

        vertices.push_back(vertex);
    }
    // process indices
    for (unsigned int i = 0; i < mesh->mNumFaces; i++)
    {
        aiFace face = mesh->mFaces[i];
        for (unsigned int j = 0; j < face.mNumIndices; j++)
            indices.push_back(face.mIndices[j]);
    }
    // process material
    //std::cout<<"material index "<<mesh->mMaterialIndex<<std::endl;
    if (mesh->mMaterialIndex >= 0)
    {
        aiMaterial *material = scene->mMaterials[mesh->mMaterialIndex];
        diffuseMaps = loadMaterialTextures(material,
                                           aiTextureType_DIFFUSE);
        specularMaps = loadMaterialTextures(material,
                                            aiTextureType_SPECULAR);
    }
    if (diffuseMaps.size() == 0)
    {
        if (diffuseMapPath != "")
            diffuseMaps.push_back(Texture::createTextureFromImage(diffuseMapPath));
        else if (color != glm::vec4(-1, -1, -1, -1))
            diffuseMaps.push_back(Texture::createTextureFromColor(colorName, 64, 64, color));
        else
            diffuseMaps.push_back(Texture::createTextureFromColor("Gray", 64, 64, glm::vec4(125, 125, 125, 255)));
    }
    if (specularMaps.size() == 0)
    {
        if (specularMapPath != "")
            specularMaps.push_back(Texture::createTextureFromImage(specularMapPath));
        else
            specularMaps.push_back(Texture::createTextureFromColor("Gray", 64, 64, glm::vec4(125, 125, 125, 255)));
    }
    Mesh *m = new Mesh(vertices, indices, diffuseMaps, specularMaps, imageCount, node);
    return m;
}

std::vector<Texture *> Model::loadMaterialTextures(aiMaterial *mat, aiTextureType type)
{
    std::vector<Texture *> textures;
    auto count = mat->GetTextureCount(type);
    //std::cout<<"texture count "<< count <<std::endl;
    for (unsigned int i = 0; i < count; i++)
    {
        aiString str;
        mat->GetTexture(type, i, &str);
        Texture *texture;
        std::string textPath = str.C_Str();
        std::string pathBegin = textPath.substr(0, 3);
        if (pathBegin == "../" || pathBegin == "..\\")
        {
            textPath.replace(0, 3, "../textures/");
        }
        else
        {
            textPath.insert(0, "../textures/");
        }
        for (int i = 0; i < textPath.size(); i++)
        {
            if (textPath[i] == '\\')
                textPath[i] = '/';
        }
        texture = Texture::createTextureFromImage(textPath);
        textures.push_back(texture);
    }
    return textures;
}

void Model::drawModel(std::vector<VkDescriptorSet> combinedDescriptorSets, VkCommandBuffer *commandBuffer, VkPipelineLayout pipelineLayout, int index)
{
    for (int i = 0; i < meshes.size(); i++)
    {
        meshes[i]->drawMesh(combinedDescriptorSets, commandBuffer, pipelineLayout, index);
    }
}

void Model::moveModel(glm::vec3 changeInPos)
{
    modelMat = glm::translate(modelMat, changeInPos);
}

void Model::scaleModel(glm::vec3 factor)
{
    modelMat = glm::scale(modelMat, factor);
}

void Model::rotateModel(glm::vec3 rotation)
{
    modelMat = glm::rotate(modelMat, glm::radians(rotation.x), glm::vec3(1, 0, 0));
    modelMat = glm::rotate(modelMat, glm::radians(rotation.y), glm::vec3(0, 1, 0));
    modelMat = glm::rotate(modelMat, glm::radians(rotation.z), glm::vec3(0, 0, 1));
}

void Model::updateModelMat(uint32_t currentImage, glm::mat4 projection, glm::mat4 view)
{
    rotateModel(rotation);
    moveModel(velocity);
    UniformBufferObject ubo{};
    ubo.modelView = view * getModelMat();
    ubo.proj = projection;
    ubo.view = view;
    ubo.normMatrix = transpose(inverse(ubo.modelView));
    for (int i = 0; i < meshes.size(); i++)
    {
        meshes[i]->updateUniformBuffers(ubo, currentImage);
    }
}

glm::vec4 Model::getModelPos()
{
    return glm::vec4(modelMat[3][0], modelMat[3][1], modelMat[3][2], 1);
}

void Model::cleanupMemory()
{
    for (int i = 0; i < meshes.size(); i++)
    {
        meshes[i]->cleanupMemory();
    }
}
