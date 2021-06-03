
#pragma once

#include "vkx/raii.hpp"
namespace vkx {

// TODO: The below code need to be moved. The code is just for template.
auto ValidateInstanceLayers(std::vector<std::string> const &required) -> bool;
auto ValidateInstanceExtensions(std::vector<std::string> const &required)
    -> bool;
auto ValidateDeviceExtensions(VkPhysicalDevice device,
			      std::vector<std::string> const &required) -> bool;

auto GetRequiredInstanceLayers() -> std::vector<std::string>;

auto GetRequiredInstanceExtensions() -> std::vector<std::string>;

auto GetRequiredDeviceExtension() -> std::vector<std::string>;

// --------------

auto GetWindowExtent(GLFWwindow *window) -> VkExtent2D;

auto GetPhysicalDeviceSurfaceFormats(VkPhysicalDevice device,
				     VkSurfaceKHR surface)
    -> std::vector<VkSurfaceFormatKHR>;

auto GetPhysicalDeviceSurfacePresentModes(VkPhysicalDevice device,
					  VkSurfaceKHR surface)
    -> std::vector<VkPresentModeKHR>;

auto GetSwapcahinImages(VkDevice device, VkSwapchainKHR swapchain)
    -> std::vector<VkImage>;

auto EnumerateInstanceLayerProperties() -> std::vector<VkLayerProperties>;

auto EnumerateInstanceExtensionProperties()
    -> std::vector<VkExtensionProperties>;

auto EnumerateDeviceExtnsionProperties(VkPhysicalDevice device)
    -> std::vector<VkExtensionProperties>;

auto EnumeratePysicalDevices(VkInstance instance)
    -> std::vector<VkPhysicalDevice>;

auto GetPhysicalDeviceQueueFamilyProperties(VkPhysicalDevice device)
    -> std::vector<VkQueueFamilyProperties>;

auto ValidatePhysicalDevice(VkPhysicalDevice device) -> bool;

auto ChoosePhysicalDevice(VkInstance instance, VkSurfaceKHR surface)
    -> VkPhysicalDevice;

auto ChoosePhysicalDevice(VkInstance instance, VkSurfaceKHR surface,
			  std::vector<std::string> const &requiredExtensions)
    -> VkPhysicalDevice;
} // namespace vkx
