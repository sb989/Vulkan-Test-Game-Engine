#ifndef __VTGE_PIPELINE_HPP__
#define __VTGE_PIPELINE_HPP__
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <vector>
#include <string>
#include <array>
class Swapchain;
class Pipeline
{
public:
    Pipeline(std::string vertPath, std::string fragPath, Swapchain *swapchain,
             VkRenderPass *renderPass, VkDescriptorSetLayout *descriptorSetLayout, uint32_t setLayoutCount);
    ~Pipeline();
    VkPipelineLayout getPipelineLayout();
    VkPipeline *getPipeline();
    //static std::vector <VkPipeline> pipelines;

private:
    VkPipelineLayout pipelineLayout;
    VkPipeline pipeline;
    VkDescriptorSetLayout *descriptorSetLayout;
    Swapchain *swapchain;
    VkRenderPass *renderPass;
    std::array<VkPipelineShaderStageCreateInfo, 2> shaderStages;
    std::string vertFilePath, fragFilePath;
    uint32_t setLayoutCount;

    void createPipeline();
    void createPipelineLayout();
    /**
         * @brief creates color blending
         */
    VkPipelineColorBlendStateCreateInfo createColorBlending();

    /**
         * @brief creates the multi sampler
         */
    VkPipelineMultisampleStateCreateInfo createMultiSampling();

    /**
         * @brief creates the resterizer
         */
    VkPipelineRasterizationStateCreateInfo createRasterizer();

    /**
         * @brief creates the view port info
         */
    VkPipelineViewportStateCreateInfo createViewPort();
    /**
         * @brief creates the input assembly
         */
    VkPipelineInputAssemblyStateCreateInfo createInputAssembly();
    /**
         * @brief creates the vertex input info
         */
    VkPipelineVertexInputStateCreateInfo *createVertexInputInfo();
    /**
         * @brief creates the depth stencil
         */
    VkPipelineDepthStencilStateCreateInfo createDepthStencil();

    /**
         * @brief loads a shader file and creates a shader module; adds the module to a class list
         * @param vertFilePath file path to the vertex shader file
         * @param fragFilePath file path to the fragment shader file
         */
    void loadShaderModule(std::string vertFilePath, std::string fragFilePath);
    /**
         * @brief creates a VkShaderModule object
         * @param code the shader code that is inputted as a vector of chars
         * @return returns the VkShaderModule that is created or throws a runtime error if it fails
         */
    VkShaderModule createShaderModule(const std::vector<char> &code);
};
#endif