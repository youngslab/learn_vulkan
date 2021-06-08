#pragma once

#include <vkx/raii.hpp>
#include <vulkan/vulkan_core.h>

namespace vkx {

namespace helper {

// without parameters, it provides default structure.
auto MakePipelineRasterizationStateCreateInfo()
    -> VkPipelineRasterizationStateCreateInfo;

auto MakePipelineInputAssemblyStateCreateInfo()
    -> VkPipelineInputAssemblyStateCreateInfo;

auto MakePipelineMultisampleStateCreateInfo()
    -> VkPipelineMultisampleStateCreateInfo;

auto MakePipelineColorBlendAttachmentState()
    -> VkPipelineColorBlendAttachmentState;

// it has some parateters which it depends on at minimum
auto MakePipelineColorBlendStateCreateInfo(
    std::vector<VkPipelineColorBlendAttachmentState> const &attachments)
    -> VkPipelineColorBlendStateCreateInfo;

auto MakePipelineDepthStencilStateCreateInfo()
    -> VkPipelineDepthStencilStateCreateInfo;

} // namespace helper

// Template functions to provide default implementation
// Which includes "CreateInfo"
auto CreateInstance(std::string const &appName) -> Instance;
auto CreateInstance(std::string const &appName,
		    std::vector<std::string> const &extensions,
		    std::vector<std::string> const &layers) -> Instance;

auto CreateDebugReportCallback(Instance instance,
			       PFN_vkDebugReportCallbackEXT callback)
    -> DebugReportCallback;

auto CreateDevice(VkPhysicalDevice physicalDevice, int graphicQueueIndex,
		  int presentationQueueIndex) -> Device;

auto CreateSwapchain(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface,
		     Device device, VkExtent2D prefered) -> Swapchain;

auto CreateRenderPass(Device const &device, VkFormat const &swapchainFormat)
    -> RenderPass;

auto CreateImageView(Device const &device, VkImage image, VkFormat format,
		     VkImageAspectFlags flags) -> ImageView;

auto CreateDescriptorSetLayout(Device const &device) -> DescriptorSetLayout;

// Descriptor Set Layout Templates
auto CreateDescriptorSetLayout(
    Device const &device,
    std::vector<VkDescriptorSetLayoutBinding> const &bindings)
    -> DescriptorSetLayout;

auto CreateDescriptorPool(Device const &device, uint32_t maxSets,
			  std::vector<VkDescriptorPoolSize> poolSizes)
    -> DescriptorPool;

auto CreateShaderModule(Device const &device, std::vector<char> const &code)
    -> ShaderModule;

auto CreateShaderModule(Device const &device, std::string filepath)
    -> ShaderModule;

// --------------------------_--------------------------------------------
inline auto MakeFragmentDescriptorSetLayoutBinding(
    uint32_t binding, VkDescriptorType descriptorType,
    uint32_t descriptorCount = 1) -> VkDescriptorSetLayoutBinding {
  return {binding, descriptorType, descriptorCount,
	  VK_SHADER_STAGE_FRAGMENT_BIT};
}

inline auto MakeVertexDescriptorSetLayoutBinding(
    uint32_t binding, VkDescriptorType descriptorType,
    uint32_t descriptorCount = 1) -> VkDescriptorSetLayoutBinding {
  return {binding, descriptorType, descriptorCount, VK_SHADER_STAGE_VERTEX_BIT};
}

auto MakeDescriptorPoolSize(VkDescriptorType type, uint32_t count)
    -> VkDescriptorPoolSize;

inline auto MakePipelineShaderStageCreateInfo(VkShaderStageFlagBits stage,
					      VkShaderModule shaderModule)
    -> VkPipelineShaderStageCreateInfo {
  return {VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
	  nullptr,
	  0,
	  stage,
	  shaderModule,
	  "main",
	  nullptr};
}

auto MakePipelineVertexInputStateCreateInfo(
    std::vector<VkVertexInputBindingDescription> const &bindings,
    std::vector<VkVertexInputAttributeDescription> const &attribs)
    -> VkPipelineVertexInputStateCreateInfo;

auto MakeVertexInputBindindingDescription(
    uint32_t binding, uint32_t stride,
    VkVertexInputRate inputRate = VK_VERTEX_INPUT_RATE_VERTEX)
    -> VkVertexInputBindingDescription;

auto MakeVertexInputAttributeDescription(uint32_t location, uint32_t binding,
					 VkFormat format, uint32_t offset)
    -> VkVertexInputAttributeDescription;

auto MakeViewport(float x, float y, float width, float height,
		  float minDepth = 0.0f, float maxDepth = 1.0f) -> VkViewport;

auto MakeScissor(VkOffset2D offset, VkExtent2D extent) -> VkRect2D;

auto MakePipelineViewportStateCreateInfo(std::vector<VkViewport> viewports,
					 std::vector<VkRect2D> scissors)
    -> VkPipelineViewportStateCreateInfo;

inline auto MakePipeineRasterizationStateCreateInfo(
    VkPipelineRasterizationStateCreateFlags flags, VkBool32 depthClampEnable,
    VkBool32 rasterizerDiscardEnable, VkPolygonMode polygonMode,
    VkCullModeFlags cullMode, VkFrontFace frontFace, VkBool32 depthBiasEnable,
    float depthBiasConstantFactor, float depthBiasClamp,
    float depthBiasSlopeFactor, float lineWidth)
    -> VkPipelineRasterizationStateCreateInfo {
  return {VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
	  nullptr,
	  flags,
	  depthClampEnable,
	  rasterizerDiscardEnable,
	  polygonMode,
	  cullMode,
	  frontFace,
	  depthBiasEnable,
	  depthBiasClamp,
	  depthBiasSlopeFactor,
	  lineWidth};
}

inline auto MakePipelineMultisampleStateCreateInfo(
    VkPipelineMultisampleStateCreateFlags flags,
    VkSampleCountFlagBits rasterizationSamples, VkBool32 sampleShadingEnable,
    float minSampleShading, const VkSampleMask *pSampleMask,
    VkBool32 alphaToCoverageEnable, VkBool32 alphaToOneEnable)
    -> VkPipelineMultisampleStateCreateInfo {
  return {VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
	  nullptr,
	  flags,
	  rasterizationSamples,
	  sampleShadingEnable,
	  minSampleShading,
	  pSampleMask,
	  alphaToCoverageEnable,
	  alphaToOneEnable};
}

auto MakePipelineColorBlendStateCreateInfo(
    std::vector<VkPipelineColorBlendAttachmentState> const &attachments)
    -> VkPipelineColorBlendStateCreateInfo;
// ------------------------------------------------------------------------

auto ChooseGraphicsQueueIndex(VkPhysicalDevice device) -> int32_t;
auto ChoosePresentQueueIndex(VkPhysicalDevice device, VkSurfaceKHR surface)
    -> int32_t;

auto ChooseSurfaceFormat(std::vector<VkSurfaceFormatKHR> const &formats)
    -> VkSurfaceFormatKHR;
auto ChooseSurfaceFormat(std::vector<VkSurfaceFormatKHR> const &formats,
			 VkSurfaceFormatKHR prefered) -> VkSurfaceFormatKHR;

auto ChoosePresentMode(std::vector<VkPresentModeKHR> const &modes)
    -> VkPresentModeKHR;
auto ChoosePresentMode(std::vector<VkPresentModeKHR> const &modes,
		       VkPresentModeKHR prefered) -> VkPresentModeKHR;

auto ChooseSwapExtent(VkSurfaceCapabilitiesKHR cap, VkExtent2D prefered)
    -> VkExtent2D;
} // namespace vkx
