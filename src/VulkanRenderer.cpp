#include "VulkanRenderer.h"
#include "VulkanValidation.h"
#include <algorithm>
#include <stdexcept>
#include <iostream>

#include <vkx/tapi.hpp>
#include <vkx/util.hpp>
#include <vkx/raii.hpp>
#include <vulkan/vulkan_core.h>

VulkanRenderer::VulkanRenderer() {}

int VulkanRenderer::init(vkx::Window const &window) {
  this->window = window;

  try {
    instance = vkx::CreateInstance("VulkanApp");
    // callback = vkx::CreateDebugReportCallback(instance, debugCallback);
    // createSurface();
    surface = vkx::Surface::Create(instance, window, nullptr);
    getPhysicalDevice();
    createLogicalDevice();
    createSwapChain();
    createSwapchainImages();
    renderPass =
	vkx::CreateRenderPass(mainDevice.logicalDevice, swapchain.GetFormat());
    createDescriptorSetLayout();

    createPushConstantRange();

    createGraphicsPipeline();
    createColourBufferImage();
    createDepthBufferImage();
    createFramebuffers();
    createCommandPool();
    createCommandBuffers();
    createTextureSampler();
    // allocateDynamicBufferTransferSpace();
    createUniformBuffers();
    createDescriptorPool();
    createDescriptorSets();
    createInputDescriptorSets();
    createSynchronisation();

    auto swapchainExtent = swapchain.GetExtent();

    uboViewProjection.projection = glm::perspective(
	glm::radians(45.0f),
	(float)swapchainExtent.width / (float)swapchainExtent.height, 0.1f,
	100.0f);
    uboViewProjection.view =
	glm::lookAt(glm::vec3(10.0f, 0.0f, 90.0f), glm::vec3(0.0f, 0.0f, -2.0f),
		    glm::vec3(0.0f, 1.0f, 0.0f));

    uboViewProjection.projection[1][1] *= -1;

    // Create our default "no texture" texture
    createTexture("plain.png");
  } catch (const std::runtime_error &e) {
    printf("ERROR: %s\n", e.what());
    return EXIT_FAILURE;
  }

  return 0;
}

void VulkanRenderer::updateModel(int modelId, glm::mat4 newModel) {
  if (modelId >= modelList.size())
    return;

  modelList[modelId].setModel(newModel);
}

void VulkanRenderer::draw() {
  // -- GET NEXT IMAGE --
  // Wait for given fence to signal (open) from last draw before continuing
  vkWaitForFences(mainDevice.logicalDevice, 1, &drawFences[currentFrame],
		  VK_TRUE, std::numeric_limits<uint64_t>::max());
  // Manually reset (close) fences
  vkResetFences(mainDevice.logicalDevice, 1, &drawFences[currentFrame]);

  // Get index of next image to be drawn to, and signal semaphore when ready to
  // be drawn to
  uint32_t imageIndex;
  vkAcquireNextImageKHR(
      mainDevice.logicalDevice, swapchain, std::numeric_limits<uint64_t>::max(),
      imageAvailable[currentFrame], VK_NULL_HANDLE, &imageIndex);

  recordCommands(imageIndex);
  updateUniformBuffers(imageIndex);

  // -- SUBMIT COMMAND BUFFER TO RENDER --
  // Queue submission information
  VkSubmitInfo submitInfo = {};
  submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
  submitInfo.waitSemaphoreCount = 1; // Number of semaphores to wait on
  submitInfo.pWaitSemaphores =
      &imageAvailable[currentFrame]; // List of semaphores to wait on
  VkPipelineStageFlags waitStages[] = {
      VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
  submitInfo.pWaitDstStageMask = waitStages; // Stages to check semaphores at
  submitInfo.commandBufferCount = 1; // Number of command buffers to submit
  submitInfo.pCommandBuffers =
      &commandBuffers[imageIndex];     // Command buffer to submit
  submitInfo.signalSemaphoreCount = 1; // Number of semaphores to signal
  submitInfo.pSignalSemaphores =
      &renderFinished[currentFrame]; // Semaphores to signal when command buffer
				     // finishes

  // Submit command buffer to queue
  VkResult result =
      vkQueueSubmit(graphicsQueue, 1, &submitInfo, drawFences[currentFrame]);
  if (result != VK_SUCCESS) {
    throw std::runtime_error("Failed to submit Command Buffer to Queue!");
  }

  // -- PRESENT RENDERED IMAGE TO SCREEN --
  VkPresentInfoKHR presentInfo = {};
  presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
  presentInfo.waitSemaphoreCount = 1; // Number of semaphores to wait on
  presentInfo.pWaitSemaphores =
      &renderFinished[currentFrame];   // Semaphores to wait on
  presentInfo.swapchainCount = 1;      // Number of swapchains to present to
  presentInfo.pSwapchains = swapchain; // Swapchains to present images to
  presentInfo.pImageIndices =
      &imageIndex; // Index of images in swapchains to present

  // Present image
  result = vkQueuePresentKHR(presentationQueue, &presentInfo);
  if (result != VK_SUCCESS) {
    throw std::runtime_error("Failed to present Image!");
  }

  // Get next frame (use % swapchainImages.size() to keep value below
  // swapchainImages.size())
  currentFrame = (currentFrame + 1) % MAX_FRAME_DRAWS;
}

void VulkanRenderer::cleanup() {
  // Wait until no actions being run on device before destroying
  vkDeviceWaitIdle(mainDevice.logicalDevice);

  //_aligned_free(modelTransferSpace);

  for (size_t i = 0; i < modelList.size(); i++) {
    modelList[i].destroyMeshModel();
  }

  vkDestroySampler(mainDevice.logicalDevice, textureSampler, nullptr);

  for (size_t i = 0; i < textureImages.size(); i++) {
    vkDestroyImageView(mainDevice.logicalDevice, textureImageViews[i], nullptr);
    vkDestroyImage(mainDevice.logicalDevice, textureImages[i], nullptr);
    vkFreeMemory(mainDevice.logicalDevice, textureImageMemory[i], nullptr);
  }

  for (size_t i = 0; i < depthBufferImage.size(); i++) {
    vkDestroyImageView(mainDevice.logicalDevice, depthBufferImageView[i],
		       nullptr);
    vkDestroyImage(mainDevice.logicalDevice, depthBufferImage[i], nullptr);
    vkFreeMemory(mainDevice.logicalDevice, depthBufferImageMemory[i], nullptr);
  }

  for (size_t i = 0; i < colourBufferImage.size(); i++) {
    vkDestroyImageView(mainDevice.logicalDevice, colourBufferImageView[i],
		       nullptr);
    vkDestroyImage(mainDevice.logicalDevice, colourBufferImage[i], nullptr);
    vkFreeMemory(mainDevice.logicalDevice, colourBufferImageMemory[i], nullptr);
  }

  for (size_t i = 0; i < swapchainImages.size(); i++) {
    vkDestroyBuffer(mainDevice.logicalDevice, vpUniformBuffer[i], nullptr);
    vkFreeMemory(mainDevice.logicalDevice, vpUniformBufferMemory[i], nullptr);
    // vkDestroyBuffer(mainDevice.logicalDevice, modelDUniformBuffer[i],
    // nullptr); vkFreeMemory(mainDevice.logicalDevice,
    // modelDUniformBufferMemory[i], nullptr);
  }
  for (size_t i = 0; i < MAX_FRAME_DRAWS; i++) {
    vkDestroySemaphore(mainDevice.logicalDevice, renderFinished[i], nullptr);
    vkDestroySemaphore(mainDevice.logicalDevice, imageAvailable[i], nullptr);
    vkDestroyFence(mainDevice.logicalDevice, drawFences[i], nullptr);
  }
  vkDestroyCommandPool(mainDevice.logicalDevice, graphicsCommandPool, nullptr);
  for (auto framebuffer : swapChainFramebuffers) {
    vkDestroyFramebuffer(mainDevice.logicalDevice, framebuffer, nullptr);
  }
  vkDestroyPipeline(mainDevice.logicalDevice, secondPipeline, nullptr);
  vkDestroyPipelineLayout(mainDevice.logicalDevice, secondPipelineLayout,
			  nullptr);
  vkDestroyPipeline(mainDevice.logicalDevice, graphicsPipeline, nullptr);
  vkDestroyPipelineLayout(mainDevice.logicalDevice, pipelineLayout, nullptr);
}

VulkanRenderer::~VulkanRenderer() {}

// void VulkanRenderer::createDebugCallback() {
//// Only create callback if validation enabled
// if (!validationEnabled)
// return;

// VkDebugReportCallbackCreateInfoEXT callbackCreateInfo = {};
// callbackCreateInfo.sType =
// VK_STRUCTURE_TYPE_DEBUG_REPORT_CALLBACK_CREATE_INFO_EXT;
// callbackCreateInfo.flags =
// VK_DEBUG_REPORT_ERROR_BIT_EXT |
// VK_DEBUG_REPORT_WARNING_BIT_EXT; // Which validation reports should
//// initiate callback
// callbackCreateInfo.pfnCallback =
// debugCallback; // Pointer to callback function itself

//// Create debug callback with custom create function
// VkResult result = CreateDebugReportCallbackEXT(instance, &callbackCreateInfo,
// nullptr, &callback);
// if (result != VK_SUCCESS) {
// throw std::runtime_error("Failed to create Debug Callback!");
//}
//}

void VulkanRenderer::createLogicalDevice() {

  // 1.Choose Graphics queue
  auto graphicQueueIndex =
      vkx::ChooseGraphicsQueueIndex(mainDevice.physicalDevice);

  // 2. Choose Presentation Queue
  auto presentQueueIndex =
      vkx::ChoosePresentQueueIndex(mainDevice.physicalDevice, surface);

  std::cout << "g: " << graphicQueueIndex << ", p: " << presentQueueIndex
	    << std::endl;

  // 3. Create Device
  mainDevice.logicalDevice = vkx::CreateDevice(
      mainDevice.physicalDevice, graphicQueueIndex, presentQueueIndex);

  // 4. Get DeviceQueues from a VkDevice
  vkGetDeviceQueue(mainDevice.logicalDevice, graphicQueueIndex, 0,
		   &graphicsQueue);
  vkGetDeviceQueue(mainDevice.logicalDevice, presentQueueIndex, 0,
		   &presentationQueue);
}

void VulkanRenderer::createSwapChain() {
  auto preferedExtent = vkx::GetWindowExtent(window);
  // TODO: Add prefered format, presentMode
  this->swapchain =
      vkx::CreateSwapchain(mainDevice.physicalDevice, surface,
			   mainDevice.logicalDevice, preferedExtent);
}

void VulkanRenderer::createSwapchainImages() {
  // 1. Get images from swapchain
  auto images = vkx::GetSwapcahinImages(mainDevice.logicalDevice, swapchain);

  // 2. Create ImageViews for each Images
  for (auto const &image : images) {
    auto imageView =
	vkx::CreateImageView(mainDevice.logicalDevice, image,
			     swapchain.GetFormat(), VK_IMAGE_ASPECT_COLOR_BIT);
    swapchainImages.push_back(imageView);
  }
}

void VulkanRenderer::createDescriptorSetLayout() {
  auto device = mainDevice.logicalDevice;

  // view projection matrix
  this->descriptorSetLayout = vkx::CreateDescriptorSetLayout(
      device, {vkx::MakeVertexDescriptorSetLayoutBinding(
		  0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER)});

  // sampler
  this->samplerSetLayout = vkx::CreateDescriptorSetLayout(
      device, {vkx::MakeFragmentDescriptorSetLayoutBinding(
		  0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER)});

  // input of color & dept
  this->inputSetLayout = vkx::CreateDescriptorSetLayout(
      device, {vkx::MakeFragmentDescriptorSetLayoutBinding(
		   0, VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT),
	       vkx::MakeFragmentDescriptorSetLayoutBinding(
		   1, VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT)});
}

void VulkanRenderer::createPushConstantRange() {
  // Define push constant values (no 'create' needed!)
  pushConstantRange.stageFlags =
      VK_SHADER_STAGE_VERTEX_BIT; // Shader stage push constant will go to
  pushConstantRange.offset =
      0; // Offset into given data to pass to push constant
  pushConstantRange.size = sizeof(Model); // Size of data being passed
}

void VulkanRenderer::createGraphicsPipeline() {
  auto device = mainDevice.logicalDevice;

  // -- SHADER STAGE CREATION INFORMATION --
  // Create Shader Modules
  auto vertShader = vkx::CreateShaderModule(device, "Shaders/vert.spv");
  auto fragShader = vkx::CreateShaderModule(device, "Shaders/frag.spv");

  auto shaderStages =
      std::vector{vkx::MakePipelineShaderStageCreateInfo(
		      VK_SHADER_STAGE_VERTEX_BIT, vertShader),
		  vkx::MakePipelineShaderStageCreateInfo(
		      VK_SHADER_STAGE_FRAGMENT_BIT, fragShader)};

  // -- VERTEX INPUT --
  auto bindingDecriptions =
      std::vector{vkx::MakeVertexInputBindindingDescription(0, sizeof(Vertex))};
  // How the data for an attribute is defined within a vertex
  auto attributeDescriptions = std::vector{
      vkx::MakeVertexInputAttributeDescription(0, 0, VK_FORMAT_R32G32B32_SFLOAT,
					       offsetof(Vertex, pos)),
      vkx::MakeVertexInputAttributeDescription(1, 0, VK_FORMAT_R32G32B32_SFLOAT,
					       offsetof(Vertex, col)),
      vkx::MakeVertexInputAttributeDescription(2, 0, VK_FORMAT_R32G32B32_SFLOAT,
					       offsetof(Vertex, tex))};

  auto vertexInputCreateInfo = vkx::MakePipelineVertexInputStateCreateInfo(
      bindingDecriptions, attributeDescriptions);

  // -- INPUT ASSEMBLY --
  auto inputAssembly = vkx::helper::MakePipelineInputAssemblyStateCreateInfo();

  // -- VIEWPORT & SCISSOR --
  // Create a viewport info struct
  auto swapchainExtent = swapchain.GetExtent();

  auto viewports = std::vector{vkx::MakeViewport(
      0.0f, 0.0f, swapchainExtent.width, swapchainExtent.height)};
  auto scissors = std::vector{vkx::MakeScissor({0, 0}, swapchainExtent)};

  auto viewportStateCreateInfo =
      vkx::MakePipelineViewportStateCreateInfo(viewports, scissors);

  // -- RASTERIZER --
  auto rasterizerCreateInfo =
      vkx::helper::MakePipelineRasterizationStateCreateInfo();

  // -- MULTISAMPLING --
  auto multisamplingCreateInfo =
      vkx::helper::MakePipelineMultisampleStateCreateInfo();

  // -- BLENDING --
  // Blending decides how to blend a new colour being written to a
  // fragment, with the old value

  auto colorStates =
      std::vector{vkx::helper::MakePipelineColorBlendAttachmentState()};

  auto colourBlendingCreateInfo =
      vkx::helper::MakePipelineColorBlendStateCreateInfo(colorStates);

  // -- PIPELINE LAYOUT --
  std::array<VkDescriptorSetLayout, 2> descriptorSetLayouts = {
      descriptorSetLayout, samplerSetLayout};

  VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo = {};
  pipelineLayoutCreateInfo.sType =
      VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
  pipelineLayoutCreateInfo.setLayoutCount =
      static_cast<uint32_t>(descriptorSetLayouts.size());
  pipelineLayoutCreateInfo.pSetLayouts = descriptorSetLayouts.data();
  pipelineLayoutCreateInfo.pushConstantRangeCount = 1;
  pipelineLayoutCreateInfo.pPushConstantRanges = &pushConstantRange;

  // Create Pipeline Layout
  VkResult result = vkCreatePipelineLayout(mainDevice.logicalDevice,
					   &pipelineLayoutCreateInfo, nullptr,
					   &pipelineLayout);
  if (result != VK_SUCCESS) {
    throw std::runtime_error("Failed to create Pipeline Layout!");
  }

  // -- DEPTH STENCIL TESTING --
  auto depthStencilCreateInfo =
      vkx::helper::MakePipelineDepthStencilStateCreateInfo();

  // -- GRAPHICS PIPELINE CREATION --
  VkGraphicsPipelineCreateInfo pipelineCreateInfo = {};
  pipelineCreateInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
  // 1. stage info
  pipelineCreateInfo.stageCount = static_cast<uint32_t>(shaderStages.size());
  pipelineCreateInfo.pStages = shaderStages.data();
  // 2.vertex input state
  pipelineCreateInfo.pVertexInputState =
      &vertexInputCreateInfo; // All the fixed function pipeline states
  pipelineCreateInfo.pInputAssemblyState = &inputAssembly;
  pipelineCreateInfo.pViewportState = &viewportStateCreateInfo;
  pipelineCreateInfo.pDynamicState = nullptr;
  pipelineCreateInfo.pRasterizationState = &rasterizerCreateInfo;
  pipelineCreateInfo.pMultisampleState = &multisamplingCreateInfo;
  pipelineCreateInfo.pColorBlendState = &colourBlendingCreateInfo;
  pipelineCreateInfo.pDepthStencilState = &depthStencilCreateInfo;
  pipelineCreateInfo.layout =
      pipelineLayout; // Pipeline Layout pipeline should use
  pipelineCreateInfo.renderPass = renderPass; // Render pass description the
					      // pipeline is compatible with
  pipelineCreateInfo.subpass = 0; // Subpass of render pass to use with pipeline

  // Pipeline Derivatives : Can create multiple pipelines that derive
  // from one another for optimisation
  pipelineCreateInfo.basePipelineHandle =
      VK_NULL_HANDLE; // Existing pipeline to derive from...
  pipelineCreateInfo.basePipelineIndex =
      -1; // or index of pipeline being created to derive from (in case
	  // creating multiple at once)

  // Create Graphics Pipeline
  result = vkCreateGraphicsPipelines(mainDevice.logicalDevice, VK_NULL_HANDLE,
				     1, &pipelineCreateInfo, nullptr,
				     &graphicsPipeline);
  if (result != VK_SUCCESS) {
    throw std::runtime_error("Failed to create a Graphics Pipeline!");
  }

  //// Destroy Shader Modules, no longer needed after Pipeline created
  // vkDestroyShaderModule(mainDevice.logicalDevice,
  // fragmentShaderModule, nullptr);
  // vkDestroyShaderModule(mainDevice.logicalDevice, vertexShaderModule,
  // nullptr);

  // CREATE SECOND PASS PIPELINE
  // Second pass shaders
  vertShader = vkx::CreateShaderModule(device, "Shaders/second_vert.spv");
  fragShader = vkx::CreateShaderModule(device, "Shaders/second_frag.spv");

  VkPipelineShaderStageCreateInfo secondShaderStages[] = {
      vkx::MakePipelineShaderStageCreateInfo(VK_SHADER_STAGE_VERTEX_BIT,
					     vertShader),
      vkx::MakePipelineShaderStageCreateInfo(VK_SHADER_STAGE_FRAGMENT_BIT,
					     fragShader)};

  // No vertex data for second pass
  vertexInputCreateInfo.vertexBindingDescriptionCount = 0;
  vertexInputCreateInfo.pVertexBindingDescriptions = nullptr;
  vertexInputCreateInfo.vertexAttributeDescriptionCount = 0;
  vertexInputCreateInfo.pVertexAttributeDescriptions = nullptr;

  // Don't want to write to depth buffer
  depthStencilCreateInfo.depthWriteEnable = VK_FALSE;

  // Create new pipeline layout
  VkPipelineLayoutCreateInfo secondPipelineLayoutCreateInfo = {};
  secondPipelineLayoutCreateInfo.sType =
      VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
  secondPipelineLayoutCreateInfo.setLayoutCount = 1;
  secondPipelineLayoutCreateInfo.pSetLayouts = inputSetLayout;
  secondPipelineLayoutCreateInfo.pushConstantRangeCount = 0;
  secondPipelineLayoutCreateInfo.pPushConstantRanges = nullptr;

  result = vkCreatePipelineLayout(mainDevice.logicalDevice,
				  &secondPipelineLayoutCreateInfo, nullptr,
				  &secondPipelineLayout);
  if (result != VK_SUCCESS) {
    throw std::runtime_error("Failed to create a Pipeline Layout!");
  }

  pipelineCreateInfo.pStages =
      secondShaderStages; // Update second shader stage list
  pipelineCreateInfo.layout =
      secondPipelineLayout;	  // Change pipeline layout for input
				  // attachment descriptor sets
  pipelineCreateInfo.subpass = 1; // Use second subpass

  // Create second pipeline
  result =
      vkCreateGraphicsPipelines(mainDevice.logicalDevice, VK_NULL_HANDLE, 1,
				&pipelineCreateInfo, nullptr, &secondPipeline);
  if (result != VK_SUCCESS) {
    throw std::runtime_error("Failed to create a Graphics Pipeline!");
  }
}

void VulkanRenderer::createColourBufferImage() {
  // Resize supported format for colour attachment
  colourBufferImage.resize(swapchainImages.size());
  colourBufferImageMemory.resize(swapchainImages.size());
  colourBufferImageView.resize(swapchainImages.size());

  // Get supported format for colour attachment
  VkFormat colourFormat =
      chooseSupportedFormat({VK_FORMAT_R8G8B8A8_UNORM}, VK_IMAGE_TILING_OPTIMAL,
			    VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);

  for (size_t i = 0; i < swapchainImages.size(); i++) {
    // Create Colour Buffer Image
    auto extent = swapchain.GetExtent();
    colourBufferImage[i] = createImage(
	extent.width, extent.height, colourFormat, VK_IMAGE_TILING_OPTIMAL,
	VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT |
	    VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT,
	VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &colourBufferImageMemory[i]);

    // Create Colour Buffer Image View
    colourBufferImageView[i] = createImageView(
	colourBufferImage[i], colourFormat, VK_IMAGE_ASPECT_COLOR_BIT);
  }
}

void VulkanRenderer::createDepthBufferImage() {
  depthBufferImage.resize(swapchainImages.size());
  depthBufferImageMemory.resize(swapchainImages.size());
  depthBufferImageView.resize(swapchainImages.size());

  // Get supported format for depth buffer
  VkFormat depthFormat = chooseSupportedFormat(
      {VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D32_SFLOAT,
       VK_FORMAT_D24_UNORM_S8_UINT},
      VK_IMAGE_TILING_OPTIMAL, VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);

  for (size_t i = 0; i < swapchainImages.size(); i++) {
    // Create Depth Buffer Image
    auto extent = swapchain.GetExtent();
    depthBufferImage[i] = createImage(
	extent.width, extent.height, depthFormat, VK_IMAGE_TILING_OPTIMAL,
	VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT |
	    VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT,
	VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &depthBufferImageMemory[i]);

    // Create Depth Buffer Image View
    depthBufferImageView[i] = createImageView(depthBufferImage[i], depthFormat,
					      VK_IMAGE_ASPECT_DEPTH_BIT);
  }
}

void VulkanRenderer::createFramebuffers() {
  // Resize framebuffer count to equal swap chain image count
  swapChainFramebuffers.resize(swapchainImages.size());

  // Create a framebuffer for each swap chain image
  for (size_t i = 0; i < swapChainFramebuffers.size(); i++) {
    std::array<VkImageView, 3> attachments = {
	swapchainImages[i], colourBufferImageView[i], depthBufferImageView[i]};

    VkFramebufferCreateInfo framebufferCreateInfo = {};
    framebufferCreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    framebufferCreateInfo.renderPass =
	renderPass; // Render Pass layout the Framebuffer will be used with
    framebufferCreateInfo.attachmentCount =
	static_cast<uint32_t>(attachments.size());
    framebufferCreateInfo.pAttachments =
	attachments.data(); // List of attachments (1:1 with Render Pass)
    auto extent = swapchain.GetExtent();
    framebufferCreateInfo.width = extent.width;	  // Framebuffer width
    framebufferCreateInfo.height = extent.height; // Framebuffer height
    framebufferCreateInfo.layers = 1;		  // Framebuffer layers

    VkResult result =
	vkCreateFramebuffer(mainDevice.logicalDevice, &framebufferCreateInfo,
			    nullptr, &swapChainFramebuffers[i]);
    if (result != VK_SUCCESS) {
      throw std::runtime_error("Failed to create a Framebuffer!");
    }
  }
}

void VulkanRenderer::createCommandPool() {
  // Get indices of queue families from device
  QueueFamilyIndices queueFamilyIndices =
      getQueueFamilies(mainDevice.physicalDevice);

  VkCommandPoolCreateInfo poolInfo = {};
  poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
  poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
  poolInfo.queueFamilyIndex =
      queueFamilyIndices.graphicsFamily; // Queue Family type that buffers
					 // from this command pool will use

  // Create a Graphics Queue Family Command Pool
  VkResult result = vkCreateCommandPool(mainDevice.logicalDevice, &poolInfo,
					nullptr, &graphicsCommandPool);
  if (result != VK_SUCCESS) {
    throw std::runtime_error("Failed to create a Command Pool!");
  }
}

void VulkanRenderer::createCommandBuffers() {
  // Resize command buffer count to have one for each framebuffer
  commandBuffers.resize(swapChainFramebuffers.size());

  VkCommandBufferAllocateInfo cbAllocInfo = {};
  cbAllocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
  cbAllocInfo.commandPool = graphicsCommandPool;
  cbAllocInfo.level =
      VK_COMMAND_BUFFER_LEVEL_PRIMARY; // VK_COMMAND_BUFFER_LEVEL_PRIMARY
				       // : Buffer you submit directly to
				       // queue. Cant be called by other
				       // buffers.
				       // VK_COMMAND_BUFFER_LEVEL_SECONARY
				       // : Buffer can't be called directly.
				       // Can be called from other buffers via
				       // "vkCmdExecuteCommands" when
				       // recording commands in primary buffer
  cbAllocInfo.commandBufferCount = static_cast<uint32_t>(commandBuffers.size());

  // Allocate command buffers and place handles in array of buffers
  VkResult result = vkAllocateCommandBuffers(
      mainDevice.logicalDevice, &cbAllocInfo, commandBuffers.data());
  if (result != VK_SUCCESS) {
    throw std::runtime_error("Failed to allocate Command Buffers!");
  }
}

void VulkanRenderer::createSynchronisation() {
  imageAvailable.resize(MAX_FRAME_DRAWS);
  renderFinished.resize(MAX_FRAME_DRAWS);
  drawFences.resize(MAX_FRAME_DRAWS);

  // Semaphore creation information
  VkSemaphoreCreateInfo semaphoreCreateInfo = {};
  semaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

  // Fence creation information
  VkFenceCreateInfo fenceCreateInfo = {};
  fenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
  fenceCreateInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

  for (size_t i = 0; i < MAX_FRAME_DRAWS; i++) {
    if (vkCreateSemaphore(mainDevice.logicalDevice, &semaphoreCreateInfo,
			  nullptr, &imageAvailable[i]) != VK_SUCCESS ||
	vkCreateSemaphore(mainDevice.logicalDevice, &semaphoreCreateInfo,
			  nullptr, &renderFinished[i]) != VK_SUCCESS ||
	vkCreateFence(mainDevice.logicalDevice, &fenceCreateInfo, nullptr,
		      &drawFences[i]) != VK_SUCCESS) {
      throw std::runtime_error("Failed to create a Semaphore and/or Fence!");
    }
  }
}

void VulkanRenderer::createTextureSampler() {
  // Sampler Creation Info
  VkSamplerCreateInfo samplerCreateInfo = {};
  samplerCreateInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
  samplerCreateInfo.magFilter =
      VK_FILTER_LINEAR; // How to render when image is magnified on screen
  samplerCreateInfo.minFilter =
      VK_FILTER_LINEAR; // How to render when image is minified on screen
  samplerCreateInfo.addressModeU =
      VK_SAMPLER_ADDRESS_MODE_REPEAT; // How to handle texture wrap in U (x)
				      // direction
  samplerCreateInfo.addressModeV =
      VK_SAMPLER_ADDRESS_MODE_REPEAT; // How to handle texture wrap in V (y)
				      // direction
  samplerCreateInfo.addressModeW =
      VK_SAMPLER_ADDRESS_MODE_REPEAT; // How to handle texture wrap in W (z)
				      // direction
  samplerCreateInfo.borderColor =
      VK_BORDER_COLOR_INT_OPAQUE_BLACK; // Border beyond texture (only workds
					// for border clamp)
  samplerCreateInfo.unnormalizedCoordinates =
      VK_FALSE; // Whether coords should be normalized (between 0 and 1)
  samplerCreateInfo.mipmapMode =
      VK_SAMPLER_MIPMAP_MODE_LINEAR;   // Mipmap interpolation mode
  samplerCreateInfo.mipLodBias = 0.0f; // Level of Details bias for mip level
  samplerCreateInfo.minLod = 0.0f; // Minimum Level of Detail to pick mip level
  samplerCreateInfo.maxLod = 0.0f; // Maximum Level of Detail to pick mip level
  samplerCreateInfo.anisotropyEnable = VK_TRUE; // Enable Anisotropy
  samplerCreateInfo.maxAnisotropy = 16;		// Anisotropy sample level

  VkResult result = vkCreateSampler(
      mainDevice.logicalDevice, &samplerCreateInfo, nullptr, &textureSampler);
  if (result != VK_SUCCESS) {
    throw std::runtime_error("Filed to create a Texture Sampler!");
  }
}

void VulkanRenderer::createUniformBuffers() {
  // ViewProjection buffer size
  VkDeviceSize vpBufferSize = sizeof(UboViewProjection);

  // Model buffer size
  // VkDeviceSize modelBufferSize = modelUniformAlignment * MAX_OBJECTS;

  // One uniform buffer for each image (and by extension, command buffer)
  vpUniformBuffer.resize(swapchainImages.size());
  vpUniformBufferMemory.resize(swapchainImages.size());
  // modelDUniformBuffer.resize(swapchainImages.size());
  // modelDUniformBufferMemory.resize(swapchainImages.size());

  // Create Uniform buffers
  for (size_t i = 0; i < swapchainImages.size(); i++) {
    createBuffer(mainDevice.physicalDevice, mainDevice.logicalDevice,
		 vpBufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
		 VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
		     VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
		 &vpUniformBuffer[i], &vpUniformBufferMemory[i]);

    /*createBuffer(mainDevice.physicalDevice, mainDevice.logicalDevice,
       modelBufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
	    VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
       VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &modelDUniformBuffer[i],
       &modelDUniformBufferMemory[i]);*/
  }
}

void VulkanRenderer::createDescriptorPool() {
  // Create Descriptor Pool
  auto device = mainDevice.logicalDevice;
  auto maxSets = static_cast<uint32_t>(swapchainImages.size());

  this->descriptorPool = vkx::CreateDescriptorPool(
      device, maxSets,
      {vkx::MakeDescriptorPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
				   vpUniformBuffer.size())});

  this->samplerDescriptorPool = vkx::CreateDescriptorPool(
      device, maxSets,
      {vkx::MakeDescriptorPoolSize(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
				   MAX_OBJECTS)});

  this->inputDescriptorPool = vkx::CreateDescriptorPool(
      device, maxSets,
      {
	  vkx::MakeDescriptorPoolSize(VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT,
				      colourBufferImageView.size()),
	  vkx::MakeDescriptorPoolSize(VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT,
				      colourBufferImageView.size()),
      });
}

void VulkanRenderer::createDescriptorSets() {
  // Resize Descriptor Set list so one for every buffer
  descriptorSets.resize(swapchainImages.size());

  std::vector<VkDescriptorSetLayout> setLayouts(swapchainImages.size(),
						descriptorSetLayout);

  // Descriptor Set Allocation Info
  VkDescriptorSetAllocateInfo setAllocInfo = {};
  setAllocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
  setAllocInfo.descriptorPool =
      descriptorPool; // Pool to allocate Descriptor Set from
  setAllocInfo.descriptorSetCount = static_cast<uint32_t>(
      swapchainImages.size()); // Number of sets to allocate
  setAllocInfo.pSetLayouts =
      setLayouts.data(); // Layouts to use to allocate sets (1:1 relationship)

  // Allocate descriptor sets (multiple)
  VkResult result = vkAllocateDescriptorSets(
      mainDevice.logicalDevice, &setAllocInfo, descriptorSets.data());
  if (result != VK_SUCCESS) {
    throw std::runtime_error("Failed to allocate Descriptor Sets!");
  }

  // Update all of descriptor set buffer bindings
  for (size_t i = 0; i < swapchainImages.size(); i++) {
    // VIEW PROJECTION DESCRIPTOR
    // Buffer info and data offset info
    VkDescriptorBufferInfo vpBufferInfo = {};
    vpBufferInfo.buffer = vpUniformBuffer[i];	    // Buffer to get data from
    vpBufferInfo.offset = 0;			    // Position of start of data
    vpBufferInfo.range = sizeof(UboViewProjection); // Size of data

    // Data about connection between binding and buffer
    VkWriteDescriptorSet vpSetWrite = {};
    vpSetWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    vpSetWrite.dstSet = descriptorSets[i]; // Descriptor Set to update
    vpSetWrite.dstBinding =
	0; // Binding to update (matches with binding on layout/shader)
    vpSetWrite.dstArrayElement = 0; // Index in array to update
    vpSetWrite.descriptorType =
	VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER; // Type of descriptor
    vpSetWrite.descriptorCount = 1;	   // Amount to update
    vpSetWrite.pBufferInfo =
	&vpBufferInfo; // Information about buffer data to bind

    // MODEL DESCRIPTOR
    // Model Buffer Binding Info
    /*VkDescriptorBufferInfo modelBufferInfo = {};
    modelBufferInfo.buffer = modelDUniformBuffer[i];
    modelBufferInfo.offset = 0;
    modelBufferInfo.range = modelUniformAlignment;

    VkWriteDescriptorSet modelSetWrite = {};
    modelSetWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    modelSetWrite.dstSet = descriptorSets[i];
    modelSetWrite.dstBinding = 1;
    modelSetWrite.dstArrayElement = 0;
    modelSetWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
    modelSetWrite.descriptorCount = 1;
    modelSetWrite.pBufferInfo = &modelBufferInfo;*/

    // List of Descriptor Set Writes
    std::vector<VkWriteDescriptorSet> setWrites = {vpSetWrite};

    // Update the descriptor sets with new buffer/binding info
    vkUpdateDescriptorSets(mainDevice.logicalDevice,
			   static_cast<uint32_t>(setWrites.size()),
			   setWrites.data(), 0, nullptr);
  }
}

void VulkanRenderer::createInputDescriptorSets() {
  // Resize array to hold descriptor set for each swap chain image
  inputDescriptorSets.resize(swapchainImages.size());

  // Fill array of layouts ready for set creation
  std::vector<VkDescriptorSetLayout> setLayouts(swapchainImages.size(),
						inputSetLayout);

  // Input Attachment Descriptor Set Allocation Info
  VkDescriptorSetAllocateInfo setAllocInfo = {};
  setAllocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
  setAllocInfo.descriptorPool = inputDescriptorPool;
  setAllocInfo.descriptorSetCount =
      static_cast<uint32_t>(swapchainImages.size());
  setAllocInfo.pSetLayouts = setLayouts.data();

  // Allocate Descriptor Sets
  VkResult result = vkAllocateDescriptorSets(
      mainDevice.logicalDevice, &setAllocInfo, inputDescriptorSets.data());
  if (result != VK_SUCCESS) {
    throw std::runtime_error(
	"Failed to allocate Input Attachment Descriptor Sets!");
  }

  // Update each descriptor set with input attachment
  for (size_t i = 0; i < swapchainImages.size(); i++) {
    // Colour Attachment Descriptor
    VkDescriptorImageInfo colourAttachmentDescriptor = {};
    colourAttachmentDescriptor.imageLayout =
	VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    colourAttachmentDescriptor.imageView = colourBufferImageView[i];
    colourAttachmentDescriptor.sampler = VK_NULL_HANDLE;

    // Colour Attachment Descriptor Write
    VkWriteDescriptorSet colourWrite = {};
    colourWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    colourWrite.dstSet = inputDescriptorSets[i];
    colourWrite.dstBinding = 0;
    colourWrite.dstArrayElement = 0;
    colourWrite.descriptorType = VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT;
    colourWrite.descriptorCount = 1;
    colourWrite.pImageInfo = &colourAttachmentDescriptor;

    // Depth Attachment Descriptor
    VkDescriptorImageInfo depthAttachmentDescriptor = {};
    depthAttachmentDescriptor.imageLayout =
	VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    depthAttachmentDescriptor.imageView = depthBufferImageView[i];
    depthAttachmentDescriptor.sampler = VK_NULL_HANDLE;

    // Depth Attachment Descriptor Write
    VkWriteDescriptorSet depthWrite = {};
    depthWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    depthWrite.dstSet = inputDescriptorSets[i];
    depthWrite.dstBinding = 1;
    depthWrite.dstArrayElement = 0;
    depthWrite.descriptorType = VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT;
    depthWrite.descriptorCount = 1;
    depthWrite.pImageInfo = &depthAttachmentDescriptor;

    // List of input descriptor set writes
    std::vector<VkWriteDescriptorSet> setWrites = {colourWrite, depthWrite};

    // Update descriptor sets
    vkUpdateDescriptorSets(mainDevice.logicalDevice,
			   static_cast<uint32_t>(setWrites.size()),
			   setWrites.data(), 0, nullptr);
  }
}

void VulkanRenderer::updateUniformBuffers(uint32_t imageIndex) {
  // Copy VP data
  void *data;
  vkMapMemory(mainDevice.logicalDevice, vpUniformBufferMemory[imageIndex], 0,
	      sizeof(UboViewProjection), 0, &data);
  memcpy(data, &uboViewProjection, sizeof(UboViewProjection));
  vkUnmapMemory(mainDevice.logicalDevice, vpUniformBufferMemory[imageIndex]);

  // Copy Model data
  /*for (size_t i = 0; i < meshList.size(); i++)
  {
	  UboModel * thisModel = (UboModel *)((uint64_t)modelTransferSpace +
  (i
  * modelUniformAlignment)); *thisModel = meshList[i].getModel();
  }

  // Map the list of model data
  vkMapMemory(mainDevice.logicalDevice, modelDUniformBufferMemory[imageIndex],
  0, modelUniformAlignment * meshList.size(), 0, &data); memcpy(data,
  modelTransferSpace, modelUniformAlignment * meshList.size());
  vkUnmapMemory(mainDevice.logicalDevice,
  modelDUniformBufferMemory[imageIndex]);*/
}

void VulkanRenderer::recordCommands(uint32_t currentImage) {
  // Information about how to begin each command buffer
  VkCommandBufferBeginInfo bufferBeginInfo = {};
  bufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

  // Information about how to begin a render pass (only needed for graphical
  // applications)
  VkRenderPassBeginInfo renderPassBeginInfo = {};
  renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
  renderPassBeginInfo.renderPass = renderPass; // Render Pass to begin
  renderPassBeginInfo.renderArea.offset = {
      0, 0}; // Start point of render pass in pixels
  renderPassBeginInfo.renderArea.extent =
      swapchain.GetExtent(); // Size of region to run render pass on (starting
			     // at offset)

  std::array<VkClearValue, 3> clearValues = {};
  clearValues[0].color = {0.0f, 0.0f, 0.0f, 1.0f};
  clearValues[1].color = {0.6f, 0.65f, 0.4f, 1.0f};
  clearValues[2].depthStencil.depth = 1.0f;

  renderPassBeginInfo.pClearValues = clearValues.data(); // List of clear values
  renderPassBeginInfo.clearValueCount =
      static_cast<uint32_t>(clearValues.size());

  renderPassBeginInfo.framebuffer = swapChainFramebuffers[currentImage];

  // Start recording commands to command buffer!
  VkResult result =
      vkBeginCommandBuffer(commandBuffers[currentImage], &bufferBeginInfo);
  if (result != VK_SUCCESS) {
    throw std::runtime_error("Failed to start recording a Command Buffer!");
  }

  // Begin Render Pass
  vkCmdBeginRenderPass(commandBuffers[currentImage], &renderPassBeginInfo,
		       VK_SUBPASS_CONTENTS_INLINE);

  // Bind Pipeline to be used in render pass
  vkCmdBindPipeline(commandBuffers[currentImage],
		    VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipeline);

  for (size_t j = 0; j < modelList.size(); j++) {
    MeshModel thisModel = modelList[j];

    auto model = thisModel.getModel();
    vkCmdPushConstants(commandBuffers[currentImage], pipelineLayout,
		       VK_SHADER_STAGE_VERTEX_BIT, // Stage to push constants to
		       0,	      // Offset of push constants to update
		       sizeof(Model), // Size of data being pushed
		       &model);	      // Actual data being pushed (can be array)

    for (size_t k = 0; k < thisModel.getMeshCount(); k++) {

      VkBuffer vertexBuffers[] = {
	  thisModel.getMesh(k)->getVertexBuffer()}; // Buffers to bind
      VkDeviceSize offsets[] = {0}; // Offsets into buffers being bound
      vkCmdBindVertexBuffers(
	  commandBuffers[currentImage], 0, 1, vertexBuffers,
	  offsets); // Command to bind vertex buffer before drawing with them

      // Bind mesh index buffer, with 0 offset and using the uint32 type
      vkCmdBindIndexBuffer(commandBuffers[currentImage],
			   thisModel.getMesh(k)->getIndexBuffer(), 0,
			   VK_INDEX_TYPE_UINT32);

      // Dynamic Offset Amount
      // uint32_t dynamicOffset = static_cast<uint32_t>(modelUniformAlignment)
      // * j;

      // "Push" constants to given shader stage directly (no buffer)

      std::array<VkDescriptorSet, 2> descriptorSetGroup = {
	  descriptorSets[currentImage],
	  samplerDescriptorSets[thisModel.getMesh(k)->getTexId()]};

      // Bind Descriptor Sets
      vkCmdBindDescriptorSets(
	  commandBuffers[currentImage], VK_PIPELINE_BIND_POINT_GRAPHICS,
	  pipelineLayout, 0, static_cast<uint32_t>(descriptorSetGroup.size()),
	  descriptorSetGroup.data(), 0, nullptr);

      // Execute pipeline
      vkCmdDrawIndexed(commandBuffers[currentImage],
		       thisModel.getMesh(k)->getIndexCount(), 1, 0, 0, 0);
    }
  }

  // Start second subpass
  vkCmdNextSubpass(commandBuffers[currentImage], VK_SUBPASS_CONTENTS_INLINE);

  vkCmdBindPipeline(commandBuffers[currentImage],
		    VK_PIPELINE_BIND_POINT_GRAPHICS, secondPipeline);
  vkCmdBindDescriptorSets(commandBuffers[currentImage],
			  VK_PIPELINE_BIND_POINT_GRAPHICS, secondPipelineLayout,
			  0, 1, &inputDescriptorSets[currentImage], 0, nullptr);
  vkCmdDraw(commandBuffers[currentImage], 3, 1, 0, 0);

  // End Render Pass
  vkCmdEndRenderPass(commandBuffers[currentImage]);

  // Stop recording to command buffer
  result = vkEndCommandBuffer(commandBuffers[currentImage]);
  if (result != VK_SUCCESS) {
    throw std::runtime_error("Failed to stop recording a Command Buffer!");
  }
}

void VulkanRenderer::getPhysicalDevice() {
  mainDevice.physicalDevice = vkx::ChoosePhysicalDevice(
      instance, surface, vkx::GetRequiredDeviceExtension());
}

void VulkanRenderer::allocateDynamicBufferTransferSpace() {
  // Calculate alignment of model data
  /*modelUniformAlignment = (sizeof(UboModel) + minUniformBufferOffset - 1)
						  & ~(minUniformBufferOffset -
  1);

  // Create space in memory to hold dynamic buffer that is aligned to our
  required alignment and holds MAX_OBJECTS modelTransferSpace = (UboModel
  *)_aligned_malloc(modelUniformAlignment * MAX_OBJECTS,
  modelUniformAlignment);*/
}

QueueFamilyIndices VulkanRenderer::getQueueFamilies(VkPhysicalDevice device) {
  QueueFamilyIndices indices;

  // Get all Queue Family Property info for the given device
  uint32_t queueFamilyCount = 0;
  vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

  std::vector<VkQueueFamilyProperties> queueFamilyList(queueFamilyCount);
  vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount,
					   queueFamilyList.data());

  // Go through each queue family and check if it has at least 1 of the
  // required types of queue
  int i = 0;
  for (const auto &queueFamily : queueFamilyList) {
    // First check if queue family has at least 1 queue in that family (could
    // have no queues) Queue can be multiple types defined through bitfield.
    // Need to bitwise AND with VK_QUEUE_*_BIT to check if has required type
    if (queueFamily.queueCount > 0 &&
	queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
      indices.graphicsFamily = i; // If queue family is valid, then get index
    }

    // Check if Queue Family supports presentation
    VkBool32 presentationSupport = false;
    vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface,
					 &presentationSupport);
    // Check if queue is presentation type (can be both graphics and
    // presentation)
    if (queueFamily.queueCount > 0 && presentationSupport) {
      indices.presentationFamily = i;
    }

    // Check if queue family indices are in a valid state, stop searching if
    // so
    if (indices.isValid()) {
      break;
    }

    i++;
  }

  return indices;
}

SwapChainDetails VulkanRenderer::getSwapChainDetails(VkPhysicalDevice device) {
  SwapChainDetails swapChainDetails;

  // -- CAPABILITIES --
  // Get the surface capabilities for the given surface on the given physical
  // device
  vkGetPhysicalDeviceSurfaceCapabilitiesKHR(
      device, surface, &swapChainDetails.surfaceCapabilities);

  // -- FORMATS --
  uint32_t formatCount = 0;
  vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, nullptr);

  // If formats returned, get list of formats
  if (formatCount != 0) {
    swapChainDetails.formats.resize(formatCount);
    vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount,
					 swapChainDetails.formats.data());
  }

  // -- PRESENTATION MODES --
  uint32_t presentationCount = 0;
  vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentationCount,
					    nullptr);

  // If presentation modes returned, get list of presentation modes
  if (presentationCount != 0) {
    swapChainDetails.presentationModes.resize(presentationCount);
    vkGetPhysicalDeviceSurfacePresentModesKHR(
	device, surface, &presentationCount,
	swapChainDetails.presentationModes.data());
  }

  return swapChainDetails;
}

VkExtent2D VulkanRenderer::chooseSwapExtent(
    const VkSurfaceCapabilitiesKHR &surfaceCapabilities) {
  // If current extent is at numeric limits, then extent can vary. Otherwise,
  // it is the size of the window.
  if (surfaceCapabilities.currentExtent.width !=
      std::numeric_limits<uint32_t>::max()) {
    return surfaceCapabilities.currentExtent;
  } else {
    // If value can vary, need to set manually

    // Get window size
    int width, height;
    glfwGetFramebufferSize(window, &width, &height);

    // Create new extent using window size
    VkExtent2D newExtent = {};
    newExtent.width = static_cast<uint32_t>(width);
    newExtent.height = static_cast<uint32_t>(height);

    // Surface also defines max and min, so make sure within boundaries by
    // clamping value
    newExtent.width = std::max(
	surfaceCapabilities.minImageExtent.width,
	std::min(surfaceCapabilities.maxImageExtent.width, newExtent.width));
    newExtent.height = std::max(
	surfaceCapabilities.minImageExtent.height,
	std::min(surfaceCapabilities.maxImageExtent.height, newExtent.height));

    return newExtent;
  }
}

VkFormat
VulkanRenderer::chooseSupportedFormat(const std::vector<VkFormat> &formats,
				      VkImageTiling tiling,
				      VkFormatFeatureFlags featureFlags) {
  // Loop through options and find compatible one
  for (VkFormat format : formats) {
    // Get properties for give format on this device
    VkFormatProperties properties;
    vkGetPhysicalDeviceFormatProperties(mainDevice.physicalDevice, format,
					&properties);

    // Depending on tiling choice, need to check for different bit flag
    if (tiling == VK_IMAGE_TILING_LINEAR &&
	(properties.linearTilingFeatures & featureFlags) == featureFlags) {
      return format;
    } else if (tiling == VK_IMAGE_TILING_OPTIMAL &&
	       (properties.optimalTilingFeatures & featureFlags) ==
		   featureFlags) {
      return format;
    }
  }

  throw std::runtime_error("Failed to find a matching format!");
}

VkImage VulkanRenderer::createImage(uint32_t width, uint32_t height,
				    VkFormat format, VkImageTiling tiling,
				    VkImageUsageFlags useFlags,
				    VkMemoryPropertyFlags propFlags,
				    VkDeviceMemory *imageMemory) {
  // CREATE IMAGE
  // Image Creation Info
  VkImageCreateInfo imageCreateInfo = {};
  imageCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
  imageCreateInfo.imageType = VK_IMAGE_TYPE_2D; // Type of image (1D, 2D, or 3D)
  imageCreateInfo.extent.width = width;		// Width of image extent
  imageCreateInfo.extent.height = height;	// Height of image extent
  imageCreateInfo.extent.depth = 1; // Depth of image (just 1, no 3D aspect)
  imageCreateInfo.mipLevels = 1;    // Number of mipmap levels
  imageCreateInfo.arrayLayers = 1;  // Number of levels in image array
  imageCreateInfo.format = format;  // Format type of image
  imageCreateInfo.tiling = tiling;  // How image data should be "tiled"
				    // (arranged for optimal reading)
  imageCreateInfo.initialLayout =
      VK_IMAGE_LAYOUT_UNDEFINED; // Layout of image data on creation
  imageCreateInfo.usage =
      useFlags; // Bit flags defining what image will be used for
  imageCreateInfo.samples =
      VK_SAMPLE_COUNT_1_BIT; // Number of samples for multi-sampling
  imageCreateInfo.sharingMode =
      VK_SHARING_MODE_EXCLUSIVE; // Whether image can be shared between queues

  // Create image
  VkImage image;
  VkResult result = vkCreateImage(mainDevice.logicalDevice, &imageCreateInfo,
				  nullptr, &image);
  if (result != VK_SUCCESS) {
    throw std::runtime_error("Failed to create an Image!");
  }

  // CREATE MEMORY FOR IMAGE

  // Get memory requirements for a type of image
  VkMemoryRequirements memoryRequirements;
  vkGetImageMemoryRequirements(mainDevice.logicalDevice, image,
			       &memoryRequirements);

  // Allocate memory using image requirements and user defined properties
  VkMemoryAllocateInfo memoryAllocInfo = {};
  memoryAllocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
  memoryAllocInfo.allocationSize = memoryRequirements.size;
  memoryAllocInfo.memoryTypeIndex = findMemoryTypeIndex(
      mainDevice.physicalDevice, memoryRequirements.memoryTypeBits, propFlags);

  result = vkAllocateMemory(mainDevice.logicalDevice, &memoryAllocInfo, nullptr,
			    imageMemory);
  if (result != VK_SUCCESS) {
    throw std::runtime_error("Failed to allocate memory for image!");
  }

  // Connect memory to image
  vkBindImageMemory(mainDevice.logicalDevice, image, *imageMemory, 0);

  return image;
}

VkImageView VulkanRenderer::createImageView(VkImage image, VkFormat format,
					    VkImageAspectFlags aspectFlags) {
  VkImageViewCreateInfo viewCreateInfo = {};
  viewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
  viewCreateInfo.image = image; // Image to create view for
  viewCreateInfo.viewType =
      VK_IMAGE_VIEW_TYPE_2D;	  // Type of image (1D, 2D, 3D, Cube, etc)
  viewCreateInfo.format = format; // Format of image data
  viewCreateInfo.components.r =
      VK_COMPONENT_SWIZZLE_IDENTITY; // Allows remapping of rgba components to
				     // other rgba values
  viewCreateInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
  viewCreateInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
  viewCreateInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;

  // Subresources allow the view to view only a part of an image
  viewCreateInfo.subresourceRange.aspectMask =
      aspectFlags; // Which aspect of image to view (e.g. COLOR_BIT for
		   // viewing colour)
  viewCreateInfo.subresourceRange.baseMipLevel =
      0; // Start mipmap level to view from
  viewCreateInfo.subresourceRange.levelCount =
      1; // Number of mipmap levels to view
  viewCreateInfo.subresourceRange.baseArrayLayer =
      0; // Start array level to view from
  viewCreateInfo.subresourceRange.layerCount =
      1; // Number of array levels to view

  // Create image view and return it
  VkImageView imageView;
  VkResult result = vkCreateImageView(mainDevice.logicalDevice, &viewCreateInfo,
				      nullptr, &imageView);
  if (result != VK_SUCCESS) {
    throw std::runtime_error("Failed to create an Image View!");
  }

  return imageView;
}

VkShaderModule
VulkanRenderer::createShaderModule(const std::vector<char> &code) {
  // Shader Module creation information
  VkShaderModuleCreateInfo shaderModuleCreateInfo = {};
  shaderModuleCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
  shaderModuleCreateInfo.codeSize = code.size(); // Size of code
  shaderModuleCreateInfo.pCode = reinterpret_cast<const uint32_t *>(
      code.data()); // Pointer to code (of uint32_t pointer type)

  VkShaderModule shaderModule;
  VkResult result =
      vkCreateShaderModule(mainDevice.logicalDevice, &shaderModuleCreateInfo,
			   nullptr, &shaderModule);
  if (result != VK_SUCCESS) {
    throw std::runtime_error("Failed to create a shader module!");
  }

  return shaderModule;
}

int VulkanRenderer::createTextureImage(std::string fileName) {
  // Load image file
  int width, height;
  VkDeviceSize imageSize;
  stbi_uc *imageData = loadTextureFile(fileName, &width, &height, &imageSize);

  // Create staging buffer to hold loaded data, ready to copy to device
  VkBuffer imageStagingBuffer;
  VkDeviceMemory imageStagingBufferMemory;
  createBuffer(mainDevice.physicalDevice, mainDevice.logicalDevice, imageSize,
	       VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
	       VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
		   VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
	       &imageStagingBuffer, &imageStagingBufferMemory);

  // Copy image data to staging buffer
  void *data;
  vkMapMemory(mainDevice.logicalDevice, imageStagingBufferMemory, 0, imageSize,
	      0, &data);
  memcpy(data, imageData, static_cast<size_t>(imageSize));
  vkUnmapMemory(mainDevice.logicalDevice, imageStagingBufferMemory);

  // Free original image data
  stbi_image_free(imageData);

  // Create image to hold final texture
  VkImage texImage;
  VkDeviceMemory texImageMemory;
  texImage = createImage(
      width, height, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_TILING_OPTIMAL,
      VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
      VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &texImageMemory);

  // COPY DATA TO IMAGE
  // Transition image to be DST for copy operation
  transitionImageLayout(
      mainDevice.logicalDevice, graphicsQueue, graphicsCommandPool, texImage,
      VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

  // Copy image data
  copyImageBuffer(mainDevice.logicalDevice, graphicsQueue, graphicsCommandPool,
		  imageStagingBuffer, texImage, width, height);

  // Transition image to be shader readable for shader usage
  transitionImageLayout(mainDevice.logicalDevice, graphicsQueue,
			graphicsCommandPool, texImage,
			VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

  // Add texture data to vector for reference
  textureImages.push_back(texImage);
  textureImageMemory.push_back(texImageMemory);

  // Destroy staging buffers
  vkDestroyBuffer(mainDevice.logicalDevice, imageStagingBuffer, nullptr);
  vkFreeMemory(mainDevice.logicalDevice, imageStagingBufferMemory, nullptr);

  // Return index of new texture image
  return textureImages.size() - 1;
}

int VulkanRenderer::createTexture(std::string fileName) {
  // Create Texture Image and get its location in array
  int textureImageLoc = createTextureImage(fileName);

  // Create Image View and add to list
  VkImageView imageView =
      createImageView(textureImages[textureImageLoc], VK_FORMAT_R8G8B8A8_UNORM,
		      VK_IMAGE_ASPECT_COLOR_BIT);
  textureImageViews.push_back(imageView);

  // Create Texture Descriptor
  int descriptorLoc = createTextureDescriptor(imageView);

  // Return location of set with texture
  return descriptorLoc;
}

int VulkanRenderer::createTextureDescriptor(VkImageView textureImage) {
  VkDescriptorSet descriptorSet;

  // Descriptor Set Allocation Info
  VkDescriptorSetAllocateInfo setAllocInfo = {};
  setAllocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
  setAllocInfo.descriptorPool = samplerDescriptorPool;
  setAllocInfo.descriptorSetCount = 1;
  setAllocInfo.pSetLayouts = samplerSetLayout;

  // Allocate Descriptor Sets
  VkResult result = vkAllocateDescriptorSets(mainDevice.logicalDevice,
					     &setAllocInfo, &descriptorSet);
  if (result != VK_SUCCESS) {
    throw std::runtime_error("Failed to allocate Texture Descriptor Sets!");
  }

  // Texture Image Info
  VkDescriptorImageInfo imageInfo = {};
  imageInfo.imageLayout =
      VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL; // Image layout when in use
  imageInfo.imageView = textureImage;		// Image to bind to set
  imageInfo.sampler = textureSampler;		// Sampler to use for set

  // Descriptor Write Info
  VkWriteDescriptorSet descriptorWrite = {};
  descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
  descriptorWrite.dstSet = descriptorSet;
  descriptorWrite.dstBinding = 0;
  descriptorWrite.dstArrayElement = 0;
  descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
  descriptorWrite.descriptorCount = 1;
  descriptorWrite.pImageInfo = &imageInfo;

  // Update new descriptor set
  vkUpdateDescriptorSets(mainDevice.logicalDevice, 1, &descriptorWrite, 0,
			 nullptr);

  // Add descriptor set to list
  samplerDescriptorSets.push_back(descriptorSet);

  // Return descriptor set location
  return samplerDescriptorSets.size() - 1;
}

int VulkanRenderer::createMeshModel(std::string modelFile) {
  // Import model "scene"
  Assimp::Importer importer;
  const aiScene *scene =
      importer.ReadFile(modelFile, aiProcess_Triangulate | aiProcess_FlipUVs |
				       aiProcess_JoinIdenticalVertices);
  if (!scene) {
    throw std::runtime_error("Failed to load model! (" + modelFile + ")");
  }

  // Get vector of all materials with 1:1 ID placement
  std::vector<std::string> textureNames = MeshModel::LoadMaterials(scene);

  // Conversion from the materials list IDs to our Descriptor Array IDs
  std::vector<int> matToTex(textureNames.size());

  // Loop over textureNames and create textures for them
  for (size_t i = 0; i < textureNames.size(); i++) {
    // If material had no texture, set '0' to indicate no texture, texture 0
    // will be reserved for a default texture
    if (textureNames[i].empty()) {
      matToTex[i] = 0;
    } else {
      // Otherwise, create texture and set value to index of new texture
      matToTex[i] = createTexture(textureNames[i]);
    }
  }

  // Load in all our meshes
  std::vector<Mesh> modelMeshes = MeshModel::LoadNode(
      mainDevice.physicalDevice, mainDevice.logicalDevice, graphicsQueue,
      graphicsCommandPool, scene->mRootNode, scene, matToTex);

  // Create mesh model and add to list
  MeshModel meshModel = MeshModel(modelMeshes);
  modelList.push_back(meshModel);

  return modelList.size() - 1;
}

stbi_uc *VulkanRenderer::loadTextureFile(std::string fileName, int *width,
					 int *height, VkDeviceSize *imageSize) {
  // Number of channels image uses
  int channels;

  // Load pixel data for image
  std::string fileLoc = "Textures/" + fileName;
  stbi_uc *image =
      stbi_load(fileLoc.c_str(), width, height, &channels, STBI_rgb_alpha);

  if (!image) {
    throw std::runtime_error("Failed to load a Texture file! (" + fileName +
			     ")");
  }

  // Calculate image size using given and known data
  *imageSize = *width * *height * 4;

  return image;
}
