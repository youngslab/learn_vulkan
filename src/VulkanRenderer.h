#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include <stdexcept>
#include <vector>
#include <set>
#include <algorithm>
#include <array>

#include "stb_image.h"

#include "Mesh.h"
#include "MeshModel.h"
#include "VulkanValidation.h"
#include "Utilities.h"

#include <vkx/raii.hpp>

class VulkanRenderer {
public:
  VulkanRenderer();

  int init(vkx::Window const &window);

  int createMeshModel(std::string modelFile);
  void updateModel(int modelId, glm::mat4 newModel);

  void draw();
  void cleanup();

  ~VulkanRenderer();

private:
  vkx::Window window;

  int currentFrame = 0;

  // Scene Objects
  std::vector<MeshModel> modelList;

  // Scene Settings
  struct UboViewProjection {
    glm::mat4 projection;
    glm::mat4 view;
  } uboViewProjection;

  // Vulkan Components
  // - Main
  // VkInstance instance;
  vkx::Instance instance;
  vkx::DebugReportCallback callback;
  // VkDebugReportCallbackEXT callback;
  struct {
    VkPhysicalDevice physicalDevice;
    vkx::Device logicalDevice;
  } mainDevice;
  VkQueue graphicsQueue;
  VkQueue presentationQueue;

  vkx::Surface surface;
  vkx::Swapchain swapchain;
  std::vector<vkx::ImageView> swapchainImages;

  std::vector<VkFramebuffer> swapChainFramebuffers;
  std::vector<VkCommandBuffer> commandBuffers;

  std::vector<vkx::Image> colourBufferImage;
  std::vector<VkDeviceMemory> colourBufferImageMemory;
  std::vector<VkImageView> colourBufferImageView;

  std::vector<vkx::Image> depthBufferImage;
  std::vector<VkDeviceMemory> depthBufferImageMemory;
  std::vector<VkImageView> depthBufferImageView;

  VkSampler textureSampler;

  // - Descriptors
  vkx::DescriptorSetLayout descriptorSetLayout;
  vkx::DescriptorSetLayout samplerSetLayout;
  vkx::DescriptorSetLayout inputSetLayout;
  VkPushConstantRange pushConstantRange;

  vkx::DescriptorPool descriptorPool;
  vkx::DescriptorPool samplerDescriptorPool;
  vkx::DescriptorPool inputDescriptorPool;

  std::vector<VkDescriptorSet> descriptorSets;
  std::vector<VkDescriptorSet> samplerDescriptorSets;
  std::vector<VkDescriptorSet> inputDescriptorSets;

  std::vector<VkBuffer> vpUniformBuffer;
  std::vector<VkDeviceMemory> vpUniformBufferMemory;

  std::vector<VkBuffer> modelDUniformBuffer;
  std::vector<VkDeviceMemory> modelDUniformBufferMemory;

  // VkDeviceSize minUniformBufferOffset;
  // size_t modelUniformAlignment;
  // UboModel * modelTransferSpace;

  // - Assets

  std::vector<vkx::Image> textureImages;
  std::vector<VkDeviceMemory> textureImageMemory;
  std::vector<VkImageView> textureImageViews;

  // - Pipeline
  vkx::Pipeline graphicsPipeline;
  vkx::PipelineLayout pipelineLayout;

  vkx::Pipeline secondPipeline;
  vkx::PipelineLayout secondPipelineLayout;

  vkx::RenderPass renderPass;

  // - Pools
  VkCommandPool graphicsCommandPool;

  // - Synchronisation
  std::vector<VkSemaphore> imageAvailable;
  std::vector<VkSemaphore> renderFinished;
  std::vector<VkFence> drawFences;

  // Vulkan Functions
  // - Create Functions
  void createInstance();
  void createDebugCallback();
  void createLogicalDevice();
  void createSurface();
  void createSwapChain();
  void createSwapchainImages();
  void createDescriptorSetLayout();

  void createPushConstantRange();
  void createGraphicsPipeline();
  void createColourBufferImage();
  void createDepthBufferImage();
  void createFramebuffers();
  void createCommandPool();
  void createCommandBuffers();
  void createSynchronisation();
  void createTextureSampler();

  void createUniformBuffers();
  void createDescriptorPool();
  void createDescriptorSets();
  void createInputDescriptorSets();

  void updateUniformBuffers(uint32_t imageIndex);

  // - Record Functions
  void recordCommands(uint32_t currentImage);

  // - Get Functions
  void getPhysicalDevice();

  // - Allocate Functions
  void allocateDynamicBufferTransferSpace();

  // -- Getter Functions
  QueueFamilyIndices getQueueFamilies(VkPhysicalDevice device);
  SwapChainDetails getSwapChainDetails(VkPhysicalDevice device);

  // -- Choose Functions
  VkSurfaceFormatKHR
  chooseBestSurfaceFormat(const std::vector<VkSurfaceFormatKHR> &formats);
  VkPresentModeKHR chooseBestPresentationMode(
      const std::vector<VkPresentModeKHR> presentationModes);
  VkExtent2D
  chooseSwapExtent(const VkSurfaceCapabilitiesKHR &surfaceCapabilities);
  VkFormat chooseSupportedFormat(const std::vector<VkFormat> &formats,
				 VkImageTiling tiling,
				 VkFormatFeatureFlags featureFlags);

  // -- Create Functions
  vkx::Image createImage(uint32_t width, uint32_t height, VkFormat format,
			 VkImageTiling tiling, VkImageUsageFlags useFlags,
			 VkMemoryPropertyFlags propFlags,
			 VkDeviceMemory *imageMemory);
  VkImageView createImageView(VkImage image, VkFormat format,
			      VkImageAspectFlags aspectFlags);
  VkShaderModule createShaderModule(const std::vector<char> &code);

  int createTextureImage(std::string fileName);
  int createTexture(std::string fileName);
  int createTextureDescriptor(VkImageView textureImage);

  // -- Loader Functions
  stbi_uc *loadTextureFile(std::string fileName, int *width, int *height,
			   VkDeviceSize *imageSize);
};

