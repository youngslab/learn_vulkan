

#include <vkx/util.hpp>
#include <vkx/raii.hpp>
#include <iostream>
#include <fstream>
#include <vulkan/vulkan_core.h>

namespace vkx {

auto validate(std::vector<std::string> const &required,
	      std::vector<std::string> const &available) -> bool {

  for (const auto &r : required) {
    bool found = false;
    for (const auto &a : available) {
      std::cout << "required: " << r << ", available: " << a << std::endl;
      if (a.compare(r) == 0) {
	found = true;
	break;
      }
    }
    if (!found)
      return false;
  }
  return true;
}

auto EnumerateInstanceLayerProperties() -> std::vector<VkLayerProperties> {
  uint32_t validationLayerCount;
  vkEnumerateInstanceLayerProperties(&validationLayerCount, nullptr);

  std::vector<VkLayerProperties> properties(validationLayerCount);
  vkEnumerateInstanceLayerProperties(&validationLayerCount, properties.data());

  return properties;
}

auto EnumerateInstanceExtensionProperties()
    -> std::vector<VkExtensionProperties> {
  auto count = 0u;
  vkEnumerateInstanceExtensionProperties(nullptr, &count, nullptr);

  std::vector<VkExtensionProperties> properties(count);
  vkEnumerateInstanceExtensionProperties(nullptr, &count, properties.data());

  return properties;
}

auto EnumerateDeviceExtnsionProperties(VkPhysicalDevice device)
    -> std::vector<VkExtensionProperties> {
  auto count = 0u;
  vkEnumerateDeviceExtensionProperties(device, nullptr, &count, nullptr);

  auto properties = std::vector<VkExtensionProperties>(count);
  vkEnumerateDeviceExtensionProperties(device, nullptr, &count,
				       properties.data());

  return properties;
}

auto EnumeratePysicalDevices(VkInstance instance)
    -> std::vector<VkPhysicalDevice> {
  uint32_t count = 0;
  vkEnumeratePhysicalDevices(instance, &count, nullptr);

  std::vector<VkPhysicalDevice> devices(count);
  vkEnumeratePhysicalDevices(instance, &count, devices.data());

  return devices;
}

auto GetPhysicalDeviceQueueFamilyProperties(VkPhysicalDevice device)
    -> std::vector<VkQueueFamilyProperties> {

  uint32_t queueFamilyCount = 0;
  vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

  std::vector<VkQueueFamilyProperties> queueFamilyList(queueFamilyCount);
  vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount,
					   queueFamilyList.data());

  return queueFamilyList;
}

auto GetPhysicalDeviceSurfaceFormats(VkPhysicalDevice device,
				     VkSurfaceKHR surface)
    -> std::vector<VkSurfaceFormatKHR> {
  uint32_t formatCount = 0;
  vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, nullptr);

  auto formats = std::vector<VkSurfaceFormatKHR>(formatCount);
  vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount,
				       formats.data());

  return formats;
}

auto GetPhysicalDeviceSurfacePresentModes(VkPhysicalDevice device,
					  VkSurfaceKHR surface)
    -> std::vector<VkPresentModeKHR> {

  uint32_t presentationCount = 0;
  vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentationCount,
					    nullptr);

  auto modes = std::vector<VkPresentModeKHR>(presentationCount);
  vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentationCount,
					    modes.data());

  return modes;
}

auto ValidateInstanceLayers(std::vector<std::string> const &required) -> bool {
  auto properties = EnumerateInstanceLayerProperties();

  std::vector<std::string> available(properties.size());
  std::transform(properties.begin(), properties.end(), available.begin(),
		 [](auto const &p) { return p.layerName; });

  return validate(required, available);
}

auto ValidateInstanceExtensions(std::vector<std::string> const &required)
    -> bool {
  auto properties = EnumerateInstanceExtensionProperties();

  std::vector<std::string> available(properties.size());
  std::transform(properties.begin(), properties.end(), available.begin(),
		 [](auto const &p) { return p.extensionName; });

  return validate(required, available);
}

auto ValidateDeviceExtensions(VkPhysicalDevice device,
			      std::vector<std::string> const &required)
    -> bool {
  auto properties = EnumerateDeviceExtnsionProperties(device);

  std::vector<std::string> available(properties.size());
  std::transform(properties.begin(), properties.end(), available.begin(),
		 [](auto const &p) { return p.extensionName; });

  return validate(required, available);
}

auto GetRequiredInstanceLayers() -> std::vector<std::string> {
  return {};
  // return {"VK_LAYER_LUNARG_standard_validation"};
}

auto GetRequiredInstanceExtensions() -> std::vector<std::string> {

  uint32_t glfwExtensionCount = 0;
  const char **glfwExtensions;
  glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

  auto extensions = std::vector<std::string>();

  // Add GLFW extensions to list of extensions
  for (size_t i = 0; i < glfwExtensionCount; i++) {
    extensions.push_back(glfwExtensions[i]);
  }

  return extensions;
}

auto GetWindowExtent(GLFWwindow *window) -> VkExtent2D {
  int width, height;
  glfwGetFramebufferSize(window, &width, &height);
  return {static_cast<uint32_t>(width), static_cast<uint32_t>(height)};
}

auto GetRequiredDeviceExtension() -> std::vector<std::string> {
  return {VK_KHR_SWAPCHAIN_EXTENSION_NAME};
}

auto ValidateDeviceFeatures(VkPhysicalDevice device) {
  VkPhysicalDeviceFeatures deviceFeatures;
  vkGetPhysicalDeviceFeatures(device, &deviceFeatures);
  return deviceFeatures.samplerAnisotropy;
}

auto ValidateDeviceQueueFamily(VkPhysicalDevice device, VkSurfaceKHR surface)
    -> bool {

  // graphics queue
  auto ps = GetPhysicalDeviceQueueFamilyProperties(device);
  auto foundGraphicsQueue = false;
  auto foundPresentationQueue = false;
  for (auto i = 0u; i < ps.size(); i++) {
    if (ps[i].queueCount < 0)
      continue;

    if (ps[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)
      foundGraphicsQueue = true;

    VkBool32 presentationSupport = false;
    vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface,
					 &presentationSupport);
    if (presentationSupport)
      foundPresentationQueue = true;
  }

  return foundGraphicsQueue && foundPresentationQueue;
}

auto ValidateDeviceSwapchain(VkPhysicalDevice device, VkSurfaceKHR surface)
    -> bool {

  auto surfaceFormats = GetPhysicalDeviceSurfaceFormats(device, surface);
  auto presetationModes = GetPhysicalDeviceSurfacePresentModes(device, surface);

  return !surfaceFormats.empty() && !presetationModes.empty();
}

auto ChoosePhysicalDevice(VkInstance instance, VkSurfaceKHR surface,
			  std::vector<std::string> const &requiredExtensions)
    -> VkPhysicalDevice {
  auto devices = EnumeratePysicalDevices(instance);
  for (auto const &device : devices) {
    if (ValidateDeviceExtensions(device, requiredExtensions) &&
	ValidateDeviceFeatures(device) &&
	ValidateDeviceQueueFamily(device, surface) &&
	ValidateDeviceSwapchain(device, surface)) {
      return device;
    }
  }
  return VK_NULL_HANDLE;
}

auto GetSwapcahinImages(VkDevice device, VkSwapchainKHR swapchain)
    -> std::vector<VkImage> {
  uint32_t count;
  vkGetSwapchainImagesKHR(device, swapchain, &count, nullptr);

  std::vector<VkImage> images(count);
  vkGetSwapchainImagesKHR(device, swapchain, &count, images.data());

  return images;
}

auto IsFormatSupported(VkPhysicalDevice device, VkFormat format,
		       VkImageTiling tiling, VkFormatFeatureFlags featureFlags)
    -> bool {

  VkFormatProperties properties;
  vkGetPhysicalDeviceFormatProperties(device, format, &properties);

  // features are not flags. ex) 0x10
  if (tiling == VK_IMAGE_TILING_LINEAR) {
    auto masked = properties.linearTilingFeatures & featureFlags;
    return masked == featureFlags;
  }

  if (tiling == VK_IMAGE_TILING_OPTIMAL) {
    auto masked = properties.optimalTilingFeatures & featureFlags;
    return masked == featureFlags;
  }

  return false;
}

auto ReadFile(const std::string &filename) -> std::vector<char> {
  std::ifstream file(filename, std::ios::binary | std::ios::ate);

  if (!file.is_open()) {
    throw std::runtime_error("Failed to open a file!");
  }

  size_t fileSize = (size_t)file.tellg();
  std::vector<char> fileBuffer(fileSize);

  file.seekg(0);
  file.read(fileBuffer.data(), fileSize);
  file.close();

  return fileBuffer;
}

} // namespace vkx
