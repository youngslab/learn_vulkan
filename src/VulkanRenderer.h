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

#include "vkx/vkx.hpp"

class VulkanRenderer {
public:
  VulkanRenderer();

  int init(vkx::Window newWindow);

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
  vkx::Instance instance;
  vkx::DebugReportCallbackEXT callback;
  struct {
    VkPhysicalDevice physicalDevice;
    vkx::Device logicalDevice;
  } mainDevice;
  VkQueue graphicsQueue;
  VkQueue presentationQueue;
  vkx::SurfaceKHR surface;
  vkx::SwapchainKHR swapchain;

  std::vector<SwapchainImage> swapChainImages;
  std::vector<vkx::Framebuffer> swapChainFramebuffers;
  std::vector<VkCommandBuffer> commandBuffers;

  vkx::Image depthBufferImage;
  VkDeviceMemory depthBufferImageMemory;
  vkx::ImageView depthBufferImageView;

  VkSampler textureSampler;

  // - Descriptors
  vkx::DescriptorSetLayout descriptorSetLayout;
  vkx::DescriptorSetLayout samplerSetLayout;
  VkPushConstantRange pushConstantRange;

  VkDescriptorPool descriptorPool;
  VkDescriptorPool samplerDescriptorPool;
  std::vector<VkDescriptorSet> descriptorSets;
  std::vector<VkDescriptorSet> samplerDescriptorSets;

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
  std::vector<vkx::ImageView> textureImageViews;

  // - Pipeline
  vkx::Pipeline graphicsPipeline;
  vkx::PipelineLayout pipelineLayout;
  vkx::RenderPass renderPass;

  // - Pools
  vkx::CommandPool graphicsCommandPool;

  // - Utility
  VkFormat swapChainImageFormat;
  VkExtent2D swapChainExtent;

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
  void createRenderPass();
  void createDescriptorSetLayout();
  void createPushConstantRange();
  void createGraphicsPipeline();
  void createDepthBufferImage();
  void createFramebuffers();
  void createCommandPool();
  void createCommandBuffers();
  void createSynchronisation();
  void createTextureSampler();

  void createUniformBuffers();
  void createDescriptorPool();
  void createDescriptorSets();

  void updateUniformBuffers(uint32_t imageIndex);

  // - Record Functions
  void recordCommands(uint32_t currentImage);

  // - Get Functions
  void getPhysicalDevice();

  // - Allocate Functions
  void allocateDynamicBufferTransferSpace();

  // - Support Functions
  // -- Checker Functions
  bool
  checkInstanceExtensionSupport(std::vector<const char *> *checkExtensions);
  bool checkDeviceExtensionSupport(VkPhysicalDevice device);
  bool checkValidationLayerSupport();
  bool checkDeviceSuitable(VkPhysicalDevice device);

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
  vkx::ImageView createImageView(VkImage image, VkFormat format,
				 VkImageAspectFlags aspectFlags);
  VkShaderModule createShaderModule(const std::vector<char> &code);

  int createTextureImage(std::string fileName);
  int createTexture(std::string fileName);
  int createTextureDescriptor(vkx::ImageView textureImage);

  // -- Loader Functions
  stbi_uc *loadTextureFile(std::string fileName, int *width, int *height,
			   VkDeviceSize *imageSize);
};

