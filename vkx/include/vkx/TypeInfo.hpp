
#pragma once

#include <GLFW/glfw3.h>
#include <vulkan/vulkan.hpp>
#include <vulkan/vulkan_core.h>

namespace vkx {

template <typename T> struct VulkanTypeInfo;

template <> struct VulkanTypeInfo<VkInstance> {
  using CreateInfo = VkInstanceCreateInfo;
  static constexpr auto Create = vkCreateInstance;
  static constexpr auto Destroy = vkDestroyInstance;
  static constexpr auto Name = "VkInstance";
};

#define DEFINE_VULKAN_DEVICE_TYPE_INFO(RESOURCE)                               \
  template <> struct VulkanTypeInfo<Vk##RESOURCE> {                            \
    using CreateInfo = Vk##RESOURCE##CreateInfo;                               \
    static constexpr auto Create = vkCreate##RESOURCE;                         \
    static constexpr auto Destroy = vkDestroy##RESOURCE;                       \
    static constexpr auto Name = "Vk" #RESOURCE;                               \
  };

#define DEFINE_VULKAN_DEVICE_TYPE_INFO_KHR(RESOURCE)                           \
  template <> struct VulkanTypeInfo<Vk##RESOURCE##KHR> {                       \
    using CreateInfo = Vk##RESOURCE##CreateInfoKHR;                            \
    static constexpr auto Create = vkCreate##RESOURCE##KHR;                    \
    static constexpr auto Destroy = vkDestroy##RESOURCE##KHR;                  \
    static constexpr auto Name = "Vk" #RESOURCE;                               \
  };

// DEFINE_VULKAN_DEVICE_TYPE_INFO(Instance);
DEFINE_VULKAN_DEVICE_TYPE_INFO(Device);
DEFINE_VULKAN_DEVICE_TYPE_INFO(Image);
DEFINE_VULKAN_DEVICE_TYPE_INFO_KHR(Swapchain);

// GLFW
// Adaptor which provides the same way to create vulkan instance
static auto CreateGLFWwindow(uint32_t w, uint32_t h, std::string title,
		      GLFWwindow **window) -> VkResult {
  glfwInit();
  glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
  glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
  *window = glfwCreateWindow(w, h, title.c_str(), nullptr, nullptr);
  if (!window)
    return VK_ERROR_UNKNOWN;
  return VK_SUCCESS;
}

static auto DeleteGLFWwindow(GLFWwindow *w) -> void {
  glfwDestroyWindow(w);
  glfwTerminate();
}

template <> struct VulkanTypeInfo<GLFWwindow *> {
  static constexpr auto Create = CreateGLFWwindow;
  static constexpr auto Destroy = DeleteGLFWwindow;
  static constexpr auto Name = "GLFWwindow";
};

} // namespace vkx

