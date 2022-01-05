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

     VkPipelineColorBlendStateCreateInfo createColorBlending();

     VkPipelineMultisampleStateCreateInfo createMultiSampling();

     VkPipelineRasterizationStateCreateInfo createRasterizer();

     VkPipelineViewportStateCreateInfo createViewPort();

     VkPipelineInputAssemblyStateCreateInfo createInputAssembly();

     VkPipelineVertexInputStateCreateInfo *createVertexInputInfo();

     VkPipelineDepthStencilStateCreateInfo createDepthStencil();

     void loadShaderModule(std::string vertFilePath, std::string fragFilePath);

     VkShaderModule createShaderModule(const std::vector<char> &code);
};
#endif