#pragma once

#include <vkx/raii.hpp>

namespace vkx {

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

auto CreateImageView(Device const &device, VkImage image, VkFormat format,
		     VkImageAspectFlags flags) -> ImageView;

// --------------------------
//
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
