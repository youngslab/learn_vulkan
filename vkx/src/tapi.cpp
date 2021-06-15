
#include <bits/stdint-uintn.h>
#include <vkx/tapi.hpp>
#include <iostream>
#include <set>
#include <vkx/util.hpp>
#include <vulkan/vulkan_core.h>

namespace vkx {

namespace helper {

auto MakePipelineDepthStencilStateCreateInfo()
    -> VkPipelineDepthStencilStateCreateInfo {
  VkPipelineDepthStencilStateCreateInfo depthStencilCreateInfo = {};
  depthStencilCreateInfo.sType =
      VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
  depthStencilCreateInfo.depthTestEnable = VK_TRUE;
  depthStencilCreateInfo.depthWriteEnable = VK_TRUE;
  depthStencilCreateInfo.depthCompareOp = VK_COMPARE_OP_LESS;
  depthStencilCreateInfo.depthBoundsTestEnable = VK_FALSE;
  depthStencilCreateInfo.stencilTestEnable = VK_FALSE; // Enable Stencil Test
  return depthStencilCreateInfo;
}
auto MakePipelineInputAssemblyStateCreateInfo()
    -> VkPipelineInputAssemblyStateCreateInfo {
  VkPipelineInputAssemblyStateCreateInfo inputAssembly = {};
  inputAssembly.sType =
      VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
  inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
  inputAssembly.primitiveRestartEnable = VK_FALSE;
  return inputAssembly;
}

auto MakePipelineRasterizationStateCreateInfo()
    -> VkPipelineRasterizationStateCreateInfo {
  return MakePipeineRasterizationStateCreateInfo(
      0, VK_FALSE, VK_FALSE, VK_POLYGON_MODE_FILL, VK_CULL_MODE_BACK_BIT,
      VK_FRONT_FACE_COUNTER_CLOCKWISE, VK_FALSE, 0.0f, 0.0f, 0.0f, 1.0f);
}

auto MakePipelineMultisampleStateCreateInfo()
    -> VkPipelineMultisampleStateCreateInfo {

  VkPipelineMultisampleStateCreateInfo createInfo = {};
  createInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
  createInfo.sampleShadingEnable = VK_FALSE;
  createInfo.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
  return createInfo;
}

auto MakePipelineColorBlendAttachmentState()
    -> VkPipelineColorBlendAttachmentState {

  VkPipelineColorBlendAttachmentState colourState = {};
  colourState.colorWriteMask =
      VK_COLOR_COMPONENT_R_BIT |
      VK_COLOR_COMPONENT_G_BIT // Colours to apply blending to
      | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
  colourState.blendEnable = VK_TRUE; // Enable blending

  // Blending uses equation: (srcColorBlendFactor * new colour)
  // colorBlendOp (dstColorBlendFactor * old colour)
  colourState.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
  colourState.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
  colourState.colorBlendOp = VK_BLEND_OP_ADD;

  // Summarised: (VK_BLEND_FACTOR_SRC_ALPHA * new colour) +
  // (VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA * old colour)
  //			   (new colour alpha * new colour) + ((1 - new
  // colour
  // alpha)
  //* old colour)
  colourState.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
  colourState.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
  colourState.alphaBlendOp = VK_BLEND_OP_ADD;

  return colourState;
}
auto MakePipelineColorBlendStateCreateInfo(
    std::vector<VkPipelineColorBlendAttachmentState> const &attachments)
    -> VkPipelineColorBlendStateCreateInfo {
  VkPipelineColorBlendStateCreateInfo createInfo = {};
  createInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
  createInfo.logicOpEnable = VK_FALSE;
  createInfo.attachmentCount = attachments.size();
  createInfo.pAttachments = attachments.data();
  return createInfo;
}

auto MakeImageCreateInfo(VkExtent3D extent, VkFormat format,
			 VkImageTiling tiling, VkImageUsageFlags useFlags)
    -> VkImageCreateInfo {

  VkImageCreateInfo crateInfo = {};
  crateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
  crateInfo.imageType = VK_IMAGE_TYPE_2D;
  crateInfo.extent = extent;
  crateInfo.mipLevels = 1;
  crateInfo.arrayLayers = 1;
  crateInfo.format = format;
  crateInfo.tiling = tiling;
  crateInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
  crateInfo.usage = useFlags;
  crateInfo.samples = VK_SAMPLE_COUNT_1_BIT;
  crateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

  return crateInfo;
}

} // namespace helper

auto CreateInstance(std::string const &appName,
		    std::vector<std::string> const &extensions,
		    std::vector<std::string> const &layers) -> Instance {

  VkApplicationInfo appInfo = {};
  appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
  appInfo.pApplicationName = appName.data();
  appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
  appInfo.pEngineName = "No Engine";
  appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
  appInfo.apiVersion = VK_API_VERSION_1_0;

  VkInstanceCreateInfo createInfo = {};
  createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
  createInfo.pApplicationInfo = &appInfo;

  // extensions
  auto extensionsConverted = std::vector<const char *>(extensions.size());
  std::transform(extensions.begin(), extensions.end(),
		 extensionsConverted.begin(),
		 [](auto const &s) { return s.data(); });
  createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
  createInfo.ppEnabledExtensionNames = extensionsConverted.data();

  // layers
  auto layersConverted = std::vector<const char *>(layers.size());
  std::transform(layers.begin(), layers.end(), layersConverted.begin(),
		 [](auto const &s) { return s.data(); });
  createInfo.enabledLayerCount = static_cast<uint32_t>(layers.size());
  createInfo.ppEnabledLayerNames = layersConverted.data();

  return Instance::Create(&createInfo, nullptr);
}

auto CreateInstance(std::string const &appName) -> Instance {

  auto extensions = vkx::GetRequiredInstanceExtensions();
  auto layers = vkx::GetRequiredInstanceLayers();

  if (!vkx::ValidateInstanceExtensions(extensions)) {
    throw std::runtime_error("Not supported required extensions!");
  }

  if (!vkx::ValidateInstanceExtensions(layers)) {
    throw std::runtime_error("Not supported required layers!");
  }

  return vkx::CreateInstance(appName, extensions, layers);
}

auto CreateDebugReportCallback(Instance instance,
			       PFN_vkDebugReportCallbackEXT callback)
    -> DebugReportCallback {
  VkDebugReportCallbackCreateInfoEXT callbackCreateInfo = {};
  callbackCreateInfo.sType =
      VK_STRUCTURE_TYPE_DEBUG_REPORT_CALLBACK_CREATE_INFO_EXT;
  callbackCreateInfo.flags =
      VK_DEBUG_REPORT_ERROR_BIT_EXT | VK_DEBUG_REPORT_WARNING_BIT_EXT;
  callbackCreateInfo.pfnCallback = callback;

  return vkx::DebugReportCallback::Create(instance, &callbackCreateInfo,
					  nullptr);
}

auto CreateDevice(VkPhysicalDevice physicalDevice, int graphicQueueIndex,
		  int presentationQueueIndex) -> Device {

  // 1. Needs Physcial Device
  // 2. QueueFamiliyIdices

  auto queueCreateInfos = std::vector<VkDeviceQueueCreateInfo>{};
  auto queueIndices = std::set<int>{graphicQueueIndex, presentationQueueIndex};

  for (int index : queueIndices) {
    VkDeviceQueueCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    createInfo.queueFamilyIndex = index;
    createInfo.queueCount = 1;
    float priority = 1.0f;
    createInfo.pQueuePriorities = &priority;
    queueCreateInfos.push_back(createInfo);
  }
  VkDeviceCreateInfo deviceCreateInfo = {};
  deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
  deviceCreateInfo.queueCreateInfoCount =
      static_cast<uint32_t>(queueCreateInfos.size());
  deviceCreateInfo.pQueueCreateInfos = queueCreateInfos.data();

  auto deviceExtensions = GetRequiredDeviceExtension();
  deviceCreateInfo.enabledExtensionCount =
      static_cast<uint32_t>(deviceExtensions.size());

  auto pDeviceExtensions = std::vector<char const *>(deviceExtensions.size());
  std::transform(deviceExtensions.begin(), deviceExtensions.end(),
		 pDeviceExtensions.begin(),
		 [](auto const &s) { return s.data(); });
  deviceCreateInfo.ppEnabledExtensionNames = pDeviceExtensions.data();

  VkPhysicalDeviceFeatures deviceFeatures = {};
  deviceFeatures.samplerAnisotropy = VK_TRUE; // Enable Anisotropy
  deviceCreateInfo.pEnabledFeatures =
      &deviceFeatures; // Physical Device features Logical Device will use

  return Device::Create(physicalDevice, &deviceCreateInfo, nullptr);
}

auto CreateSwapchain(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface,
		     Device device, VkExtent2D prefered) -> Swapchain {

  auto formats = GetPhysicalDeviceSurfaceFormats(physicalDevice, surface);
  auto surfaceFormat = ChooseSurfaceFormat(formats);

  auto presentModes =
      GetPhysicalDeviceSurfacePresentModes(physicalDevice, surface);
  auto presentMode = ChoosePresentMode(presentModes);

  VkSurfaceCapabilitiesKHR surfaceCapabilities = {};

  vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, surface,
					    &surfaceCapabilities);
  auto extent = ChooseSwapExtent(surfaceCapabilities, prefered);

  auto imageCount = surfaceCapabilities.minImageCount + 1;
  if (surfaceCapabilities.maxImageCount > 0 &&
      surfaceCapabilities.maxImageCount < imageCount) {
    imageCount = surfaceCapabilities.maxImageCount;
  }

  // Creation information for swap chain
  VkSwapchainCreateInfoKHR swapChainCreateInfo = {};
  swapChainCreateInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
  swapChainCreateInfo.surface = surface;
  swapChainCreateInfo.imageFormat = surfaceFormat.format;
  swapChainCreateInfo.imageColorSpace = surfaceFormat.colorSpace;
  swapChainCreateInfo.presentMode = presentMode;
  swapChainCreateInfo.imageExtent = extent;
  swapChainCreateInfo.minImageCount = imageCount;
  swapChainCreateInfo.imageArrayLayers = 1;
  swapChainCreateInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
  swapChainCreateInfo.preTransform = surfaceCapabilities.currentTransform;
  swapChainCreateInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
  swapChainCreateInfo.clipped = VK_TRUE;

  // How many queues?
  auto graphicsQueueInidex = ChooseGraphicsQueueIndex(physicalDevice);
  auto presentQueueIndex = ChoosePresentQueueIndex(physicalDevice, surface);

  // let images be shared between families
  if (graphicsQueueInidex != presentQueueIndex) {
    uint32_t queueFamilyIndices[] = {//
				     static_cast<uint32_t>(graphicsQueueInidex),
				     static_cast<uint32_t>(presentQueueIndex)};
    swapChainCreateInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
    swapChainCreateInfo.queueFamilyIndexCount = 2;
    swapChainCreateInfo.pQueueFamilyIndices = queueFamilyIndices;
  } else {
    swapChainCreateInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    swapChainCreateInfo.queueFamilyIndexCount = 0;
    swapChainCreateInfo.pQueueFamilyIndices = nullptr;
  }

  return Swapchain::Create(device, &swapChainCreateInfo, nullptr);
}

auto CreateImageView(Device const &device, VkImage image, VkFormat format,
		     VkImageAspectFlags aspectFlags) -> ImageView {
  VkImageViewCreateInfo viewCreateInfo = {};
  viewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
  viewCreateInfo.image = image; // Image to create view for
  viewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
  viewCreateInfo.format = format;

  viewCreateInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
  viewCreateInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
  viewCreateInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
  viewCreateInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;

  viewCreateInfo.subresourceRange.aspectMask = aspectFlags;
  viewCreateInfo.subresourceRange.baseMipLevel = 0;
  viewCreateInfo.subresourceRange.levelCount = 1;
  viewCreateInfo.subresourceRange.baseArrayLayer = 0;
  viewCreateInfo.subresourceRange.layerCount = 1;

  return ImageView::Create(device, &viewCreateInfo, nullptr);
}

namespace details {

auto CreateRenderPassAttachmentDescriptions(VkFormat color, VkFormat depth,
					    VkFormat swapchain)
    -> std::array<VkAttachmentDescription, 3> {

  // Colour Attachment (Input)
  VkAttachmentDescription colourAttachment = {};
  colourAttachment.format = color;
  colourAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
  colourAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
  colourAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
  colourAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
  colourAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
  colourAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
  colourAttachment.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

  // Depth attachment (Input)
  VkAttachmentDescription depthAttachment = {};
  depthAttachment.format = depth;
  depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
  depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
  depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
  depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
  depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
  depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
  depthAttachment.finalLayout =
      VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

  // Swapchain colour attachment
  VkAttachmentDescription swapchainColourAttachment = {};
  swapchainColourAttachment.format = swapchain;
  swapchainColourAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
  swapchainColourAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
  swapchainColourAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
  swapchainColourAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
  swapchainColourAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
  swapchainColourAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
  swapchainColourAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

  return {swapchainColourAttachment, colourAttachment, depthAttachment};
}

auto CreateSubpassDescriptions() -> std::array<VkSubpassDescription, 2> {
  std::array<VkSubpassDescription, 2> subpasses;

  // Subpass #1
  VkAttachmentReference colorAttachmentReference = {};
  colorAttachmentReference.attachment = 1;
  colorAttachmentReference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

  VkAttachmentReference depthAttachmentReference = {};
  depthAttachmentReference.attachment = 2;
  depthAttachmentReference.layout =
      VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

  subpasses[0].pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
  subpasses[0].colorAttachmentCount = 1;
  subpasses[0].pColorAttachments = &colorAttachmentReference;
  subpasses[0].pDepthStencilAttachment = &depthAttachmentReference;

  // Subpass #2
  VkAttachmentReference swapchainColourAttachmentReference = {};
  swapchainColourAttachmentReference.attachment = 0;
  swapchainColourAttachmentReference.layout =
      VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

  std::array<VkAttachmentReference, 2> inputReferences;
  inputReferences[0].attachment = 1;
  inputReferences[0].layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
  inputReferences[1].attachment = 2;
  inputReferences[1].layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

  subpasses[1].pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
  subpasses[1].colorAttachmentCount = 1;
  subpasses[1].pColorAttachments = &swapchainColourAttachmentReference;
  subpasses[1].inputAttachmentCount =
      static_cast<uint32_t>(inputReferences.size());
  subpasses[1].pInputAttachments = inputReferences.data();

  return subpasses;
}

auto CreateRenderPassDependencies() -> std::array<VkSubpassDependency, 3> {

  std::array<VkSubpassDependency, 3> subpassDependencies{};

  subpassDependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;
  subpassDependencies[0].srcStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
  subpassDependencies[0].srcAccessMask = VK_ACCESS_MEMORY_READ_BIT;

  subpassDependencies[0].dstSubpass = 0;
  subpassDependencies[0].dstStageMask =
      VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
  subpassDependencies[0].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT |
					 VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
  subpassDependencies[0].dependencyFlags = 0;

  subpassDependencies[1].srcSubpass = 0;
  subpassDependencies[1].srcStageMask =
      VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
  subpassDependencies[1].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
  subpassDependencies[1].dstSubpass = 1;
  subpassDependencies[1].dstStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
  subpassDependencies[1].dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
  subpassDependencies[1].dependencyFlags = 0;

  subpassDependencies[2].srcSubpass = 0;
  subpassDependencies[2].srcStageMask =
      VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
  subpassDependencies[2].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT |
					 VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

  subpassDependencies[2].dstSubpass = VK_SUBPASS_EXTERNAL;
  subpassDependencies[2].dstStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
  subpassDependencies[2].dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
  subpassDependencies[2].dependencyFlags = 0;

  return subpassDependencies;
}

} // namespace details

auto CreateRenderPass(Device const &device, VkFormat const &swapchainFormat)
    -> RenderPass {

  std::array<VkSubpassDescription, 2> subpasses{};

  // ATTACHMENTS
  // SUBPASS 1 ATTACHMENTS + REFERENCES (INPUT ATTACHMENTS)

  // Colour Attachment (Input)
  VkAttachmentDescription colourAttachment = {};
  colourAttachment.format = VK_FORMAT_R8G8B8A8_UNORM;
  colourAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
  colourAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
  colourAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
  colourAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
  colourAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
  colourAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
  colourAttachment.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

  // Depth attachment (Input)
  VkAttachmentDescription depthAttachment = {};
  depthAttachment.format = VK_FORMAT_D32_SFLOAT_S8_UINT;
  depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
  depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
  depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
  depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
  depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
  depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
  depthAttachment.finalLayout =
      VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

  // Colour Attachment (Input) Reference
  VkAttachmentReference colourAttachmentReference = {};
  colourAttachmentReference.attachment = 1;
  colourAttachmentReference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

  // Depth Attachment (Input) Reference
  VkAttachmentReference depthAttachmentReference = {};
  depthAttachmentReference.attachment = 2;
  depthAttachmentReference.layout =
      VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

  // Set up Subpass 1
  subpasses[0].pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
  subpasses[0].colorAttachmentCount = 1;
  subpasses[0].pColorAttachments = &colourAttachmentReference;
  subpasses[0].pDepthStencilAttachment = &depthAttachmentReference;

  // SUBPASS 2 ATTACHMENTS + REFERENCES

  // Swapchain colour attachment
  VkAttachmentDescription swapchainColourAttachment = {};
  swapchainColourAttachment.format = swapchainFormat;
  swapchainColourAttachment.samples =
      VK_SAMPLE_COUNT_1_BIT; // Number of samples to write for multisampling
  swapchainColourAttachment.loadOp =
      VK_ATTACHMENT_LOAD_OP_CLEAR; // Describes what to do with attachment
				   // before rendering
  swapchainColourAttachment.storeOp =
      VK_ATTACHMENT_STORE_OP_STORE; // Describes what to do with attachment
				    // after rendering
  swapchainColourAttachment.stencilLoadOp =
      VK_ATTACHMENT_LOAD_OP_DONT_CARE; // Describes what to do with stencil
				       // before rendering
  swapchainColourAttachment.stencilStoreOp =
      VK_ATTACHMENT_STORE_OP_DONT_CARE; // Describes what to do with stencil
					// after rendering

  // Framebuffer data will be stored as an image, but images can be given
  // different data layouts to give optimal use for certain operations
  swapchainColourAttachment.initialLayout =
      VK_IMAGE_LAYOUT_UNDEFINED; // Image data layout before render pass starts
  swapchainColourAttachment.finalLayout =
      VK_IMAGE_LAYOUT_PRESENT_SRC_KHR; // Image data layout after render pass
				       // (to change to)

  // Attachment reference uses an attachment index that refers to index in the
  // attachment list passed to renderPassCreateInfo
  VkAttachmentReference swapchainColourAttachmentReference = {};
  swapchainColourAttachmentReference.attachment = 0;
  swapchainColourAttachmentReference.layout =
      VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

  // References to attachments that subpass will take input from
  std::array<VkAttachmentReference, 2> inputReferences;
  inputReferences[0].attachment = 1;
  inputReferences[0].layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
  inputReferences[1].attachment = 2;
  inputReferences[1].layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

  // Set up Subpass 2
  subpasses[1].pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
  subpasses[1].colorAttachmentCount = 1;
  subpasses[1].pColorAttachments = &swapchainColourAttachmentReference;
  subpasses[1].inputAttachmentCount =
      static_cast<uint32_t>(inputReferences.size());
  subpasses[1].pInputAttachments = inputReferences.data();

  // SUBPASS DEPENDENCIES

  // Need to determine when layout transitions occur using subpass
  // dependencies
  std::array<VkSubpassDependency, 3> subpassDependencies;

  // Conversion from VK_IMAGE_LAYOUT_UNDEFINED to
  // VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL Transition must happen after...
  subpassDependencies[0].srcSubpass =
      VK_SUBPASS_EXTERNAL; // Subpass index (VK_SUBPASS_EXTERNAL = Special value
			   // meaning outside of renderpass)
  subpassDependencies[0].srcStageMask =
      VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT; // Pipeline stage
  subpassDependencies[0].srcAccessMask =
      VK_ACCESS_MEMORY_READ_BIT; // Stage access mask (memory access)
				 // But must happen before...
  subpassDependencies[0].dstSubpass = 0;
  subpassDependencies[0].dstStageMask =
      VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
  subpassDependencies[0].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT |
					 VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
  subpassDependencies[0].dependencyFlags = 0;

  // Subpass 1 layout (colour/depth) to Subpass 2 layout (shader read)
  subpassDependencies[1].srcSubpass = 0;
  subpassDependencies[1].srcStageMask =
      VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
  subpassDependencies[1].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
  subpassDependencies[1].dstSubpass = 1;
  subpassDependencies[1].dstStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
  subpassDependencies[1].dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
  subpassDependencies[1].dependencyFlags = 0;

  // Conversion from VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL to
  // VK_IMAGE_LAYOUT_PRESENT_SRC_KHR Transition must happen after...
  subpassDependencies[2].srcSubpass = 0;
  subpassDependencies[2].srcStageMask =
      VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
  subpassDependencies[2].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT |
					 VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
  // But must happen before...
  subpassDependencies[2].dstSubpass = VK_SUBPASS_EXTERNAL;
  subpassDependencies[2].dstStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
  subpassDependencies[2].dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
  subpassDependencies[2].dependencyFlags = 0;

  std::array<VkAttachmentDescription, 3> renderPassAttachments = {
      swapchainColourAttachment, colourAttachment, depthAttachment};

  // Create info for Render Pass
  VkRenderPassCreateInfo renderPassCreateInfo = {};
  renderPassCreateInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
  renderPassCreateInfo.attachmentCount =
      static_cast<uint32_t>(renderPassAttachments.size());
  renderPassCreateInfo.pAttachments = renderPassAttachments.data();
  renderPassCreateInfo.subpassCount = static_cast<uint32_t>(subpasses.size());
  renderPassCreateInfo.pSubpasses = subpasses.data();
  renderPassCreateInfo.dependencyCount =
      static_cast<uint32_t>(subpassDependencies.size());
  renderPassCreateInfo.pDependencies = subpassDependencies.data();

  return RenderPass::Create(device, &renderPassCreateInfo, nullptr);
}

auto CreateDescriptorSetLayout(
    Device const &device,
    std::vector<VkDescriptorSetLayoutBinding> const &bindings)
    -> DescriptorSetLayout {

  VkDescriptorSetLayoutCreateInfo createInfo{};
  createInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
  createInfo.bindingCount = static_cast<uint32_t>(bindings.size());
  createInfo.pBindings = bindings.data();
  return CreateDescriptorSetLayout(device, &createInfo, nullptr);
}

auto CreateDescriptorPool(Device const &device, uint32_t maxSets,
			  std::vector<VkDescriptorPoolSize> poolSizes)
    -> DescriptorPool {

  VkDescriptorPoolCreateInfo poolCreateInfo = {};
  poolCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
  poolCreateInfo.maxSets = maxSets;
  poolCreateInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
  poolCreateInfo.pPoolSizes = poolSizes.data();

  return CreateDescriptorPool(device, &poolCreateInfo, nullptr);
}

auto CreateShaderModule(Device const &device, std::vector<char> const &code)
    -> ShaderModule {

  // Shader Module creation information
  VkShaderModuleCreateInfo createInfo = {};
  createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
  createInfo.codeSize = code.size();
  createInfo.pCode = reinterpret_cast<const uint32_t *>(code.data());
  return CreateShaderModule(device, &createInfo, nullptr);
}

auto CreateShaderModule(Device const &device, std::string filepath)
    -> ShaderModule {
  auto code = ReadFile(filepath);
  return CreateShaderModule(device, code);
}

auto MakeDescriptorPoolSize(VkDescriptorType type, uint32_t count)
    -> VkDescriptorPoolSize {
  return VkDescriptorPoolSize{type, count};
}

auto MakePipelineVertexInputStateCreateInfo(
    std::vector<VkVertexInputBindingDescription> const &bindings,
    std::vector<VkVertexInputAttributeDescription> const &attribs)
    -> VkPipelineVertexInputStateCreateInfo {

  VkPipelineVertexInputStateCreateInfo createInfo = {};
  createInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;

  createInfo.vertexBindingDescriptionCount = bindings.size();
  createInfo.pVertexBindingDescriptions = bindings.data();

  createInfo.vertexAttributeDescriptionCount = attribs.size();
  createInfo.pVertexAttributeDescriptions = attribs.data();

  return createInfo;
}

auto MakeVertexInputBindindingDescription(uint32_t binding, uint32_t stride,
					  VkVertexInputRate inputRate)
    -> VkVertexInputBindingDescription {
  return {binding, stride, inputRate};
}

auto MakeVertexInputAttributeDescription(uint32_t location, uint32_t binding,
					 VkFormat format, uint32_t offset)
    -> VkVertexInputAttributeDescription {
  return {location, binding, format, offset};
}

auto MakeViewport(float x, float y, float width, float height, float minDepth,
		  float maxDepth) -> VkViewport {
  return {x, y, width, height, minDepth, maxDepth};
}

auto MakeScissor(VkOffset2D offset, VkExtent2D extent) -> VkRect2D {
  return {offset, extent};
}

auto MakePipelineViewportStateCreateInfo(
    std::vector<VkViewport> const &viewports,
    std::vector<VkRect2D> const &scissors)
    -> VkPipelineViewportStateCreateInfo {

  VkPipelineViewportStateCreateInfo createInfo = {};
  createInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
  createInfo.viewportCount = viewports.size();
  createInfo.pViewports = viewports.data();
  createInfo.scissorCount = scissors.size();
  createInfo.pScissors = scissors.data();
  return createInfo;
}

auto ChooseGraphicsQueueIndex(VkPhysicalDevice device) -> int32_t {
  auto properties = vkx::GetPhysicalDeviceQueueFamilyProperties(device);
  for (int i = 0; i < properties.size(); i++) {
    if (properties[i].queueCount <= 0)
      continue;
    if (properties[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)
      return i;
  }
  return -1;
}

auto ChoosePresentQueueIndex(VkPhysicalDevice device, VkSurfaceKHR surface)
    -> int32_t {
  auto properties = vkx::GetPhysicalDeviceQueueFamilyProperties(device);
  for (int i = 0; i < properties.size(); i++) {
    if (properties[i].queueCount <= 0)
      continue;

    VkBool32 presentationSupport = false;
    vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface,
					 &presentationSupport);
    if (presentationSupport)
      return i;
  }
  return -1;
}

auto ChooseSurfaceFormat(std::vector<VkSurfaceFormatKHR> const &formats)
    -> VkSurfaceFormatKHR {
  return ChooseSurfaceFormat(
      formats, {VK_FORMAT_R8G8B8A8_UNORM, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR});
}

auto ChooseSurfaceFormat(std::vector<VkSurfaceFormatKHR> const &formats,
			 VkSurfaceFormatKHR prefered) -> VkSurfaceFormatKHR {
  // If no restricion case (Undefined and Only one format)
  if (formats.size() == 1 && formats[0].format == VK_FORMAT_UNDEFINED) {
    return prefered;
  }

  // Else, search each cases to find prefered format
  for (const auto &format : formats) {
    if ((format.format == prefered.format) &&
	format.colorSpace == prefered.colorSpace) {
      return format;
    }
  }

  // If can't find optimal format, then just return first format
  return formats[0];
}

auto ChoosePresentMode(std::vector<VkPresentModeKHR> const &modes)
    -> VkPresentModeKHR {
  return ChoosePresentMode(modes, VK_PRESENT_MODE_MAILBOX_KHR);
}

auto ChoosePresentMode(std::vector<VkPresentModeKHR> const &modes,
		       VkPresentModeKHR prefered) -> VkPresentModeKHR {
  // Look for Mailbox presentation mode
  for (const auto &mode : modes) {
    if (mode == prefered) {
      return mode;
    }
  }

  // If can't find, use FIFO as Vulkan spec says it must be present
  return VK_PRESENT_MODE_FIFO_KHR;
}

// TODO: "clamp" is more prefered function name.
auto ChooseSwapExtent(VkSurfaceCapabilitiesKHR cap, VkExtent2D prefered)
    -> VkExtent2D {
  if (cap.currentExtent.width != std::numeric_limits<uint32_t>::max()) {
    return cap.currentExtent;
  }

  // Clamp prefered values based on max limitation from surface
  VkExtent2D extent2d = prefered;
  extent2d.width = std::max(cap.minImageExtent.width,
			    std::min(cap.maxImageExtent.width, extent2d.width));
  extent2d.height =
      std::max(cap.minImageExtent.height,
	       std::min(cap.maxImageExtent.height, extent2d.height));

  return extent2d;
}

} // namespace vkx
