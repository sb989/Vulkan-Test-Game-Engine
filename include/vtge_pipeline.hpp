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
              VkRenderPass *renderPass, VkDescriptorSetLayout *descriptorSetLayout, uint32_t setLayoutCount, uint32_t subpassIndex);
     Pipeline(std::string vertPath, std::string fragPath, VkRenderPass *renderPass,
              VkDescriptorSetLayout *descriptorSetLayout, uint32_t setLayoutCount, uint32_t subpassIndex);
     ~Pipeline();
     static Pipeline *createDrawingPipeline(std::string vertPath, std::string fragPath, Swapchain *swapchain,
                                            VkRenderPass *renderPass, VkDescriptorSetLayout *descriptorSetLayout,
                                            uint32_t setLayoutCount, uint32_t subpassIndex);
     static Pipeline *createShadowPipeline(std::string vertPath, std::string fragPath, VkRenderPass *renderPass,
                                           VkDescriptorSetLayout *descriptorSetLayout, uint32_t setLayoutCount, uint32_t subpassIndex);
     VkPipelineLayout getPipelineLayout();
     VkPipeline *getPipeline();
     //static std::vector <VkPipeline> pipelines;

private:
     VkPipelineLayout pipelineLayout;
     VkPipeline pipeline;
     VkDescriptorSetLayout *descriptorSetLayout;
     //Swapchain *swapchain;
     VkRenderPass *renderPass;
     VkExtent2D extent;
     std::array<VkPipelineShaderStageCreateInfo, 2> shaderStages;
     std::string vertFilePath, fragFilePath;
     uint32_t setLayoutCount;
     float viewPortWidth, viewPortHeight;

     void createPipeline(uint32_t subpassIndex);
     void createDepthOnlyPipeline(uint32_t subpassIndex);
     void createPipelineLayout();

     VkPipelineColorBlendStateCreateInfo createColorBlending();

VkPipelineMultisampleStateCreateInfo createMultiSampling(VkSampleCountFlagBits sampleCount);

     VkPipelineRasterizationStateCreateInfo createRasterizer();

     VkPipelineViewportStateCreateInfo createViewPort();

     VkPipelineInputAssemblyStateCreateInfo createInputAssembly();

     VkPipelineVertexInputStateCreateInfo *createVertexInputInfo();

     VkPipelineDepthStencilStateCreateInfo createDepthStencil();

     void loadShaderModule(std::string vertFilePath, std::string fragFilePath);

     VkShaderModule createShaderModule(const std::vector<char> &code);
};
#endif