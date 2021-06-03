
#include <bits/stdint-uintn.h>
#include <vkx/tapi.hpp>
#include <iostream>
#include <set>
#include <vkx/util.hpp>
#include <vulkan/vulkan_core.h>

namespace vkx {

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
    VkDeviceQueueCreateInfo createInfo;
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

  VkSurfaceCapabilitiesKHR surfaceCapabilities;

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
