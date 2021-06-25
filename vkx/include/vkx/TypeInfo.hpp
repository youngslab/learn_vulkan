
#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <vulkan/vulkan.hpp>
#include <vulkan/vulkan_core.h>
#include <vkx/Ext.hpp>

namespace vkx {

template <typename T> struct VulkanTypeInfo;

template <typename Resource> struct Destroyer {
  template <typename Dependency, typename CreateInfo>
  static auto Destroy2(Resource handle, Dependency dep, CreateInfo,
		       const VkAllocationCallbacks *pAllocator) {
    if (handle == VK_NULL_HANDLE)
      return;
    VulkanTypeInfo<Resource>::Destroy(dep, handle, pAllocator);
  }

  template <typename CreateInfo>
  static auto Destroy2(Resource handle, CreateInfo,
		       const VkAllocationCallbacks *pAllocator) {
    if (handle == VK_NULL_HANDLE)
      return;
    VulkanTypeInfo<Resource>::Destroy(handle, pAllocator);
  }

  static auto Destroy2(Resource handle, uint32_t, uint32_t, std::string) {
    if (handle == VK_NULL_HANDLE)
      return;
    VulkanTypeInfo<Resource>::Destroy(handle);
  }

  static auto Destroy2(Resource device, VkPhysicalDevice,
		       const VkDeviceCreateInfo *,
		       const VkAllocationCallbacks *pAllocator) {
    VulkanTypeInfo<Resource>::Destroy(device, pAllocator);
  }
};

#define DEFINE_VULKAN_TYPE_INFO(RESOURCE)                                      \
  template <> struct VulkanTypeInfo<Vk##RESOURCE> : Destroyer<Vk##RESOURCE> {  \
    using CreateInfo = Vk##RESOURCE##CreateInfo;                               \
    static constexpr auto Create = vkCreate##RESOURCE;                         \
    static constexpr auto Destroy = vkDestroy##RESOURCE;                       \
    static constexpr auto Name = "Vk" #RESOURCE;                               \
  };

#define DEFINE_VULKAN_TYPE_INFO_KHR(RESOURCE)                                  \
  template <>                                                                  \
  struct VulkanTypeInfo<Vk##RESOURCE##KHR> : Destroyer<Vk##RESOURCE##KHR> {    \
    using CreateInfo = Vk##RESOURCE##CreateInfoKHR;                            \
    static constexpr auto Create = vkCreate##RESOURCE##KHR;                    \
    static constexpr auto Destroy = vkDestroy##RESOURCE##KHR;                  \
    static constexpr auto Name = "Vk" #RESOURCE;                               \
  };

#define DEFINE_VULKAN_TYPE_INFO_EXT(RESOURCE)                                  \
  template <>                                                                  \
  struct VulkanTypeInfo<Vk##RESOURCE##EXT> : Destroyer<Vk##RESOURCE##EXT> {    \
    using CreateInfo = Vk##RESOURCE##CreateInfoEXT;                            \
    static constexpr char const CreateName[] = "vkCreate" #RESOURCE "EXT";     \
    static constexpr char const DestroyName[] = "vkDestroy" #RESOURCE "EXT";   \
    static constexpr auto Create =                                             \
	LoadExtFunction<PFN_vkCreate##RESOURCE##EXT,                           \
			VulkanTypeInfo::CreateName>();                         \
    static constexpr auto Destroy =                                            \
	LoadExtFunction<PFN_vkDestroy##RESOURCE##EXT,                          \
			VulkanTypeInfo::DestroyName>();                        \
    static constexpr auto Name = "Vk" #RESOURCE;                               \
  };

DEFINE_VULKAN_TYPE_INFO(Instance);
DEFINE_VULKAN_TYPE_INFO_EXT(DebugReportCallback);
// DEFINE_VULKAN_TYPE_INFO_KHR(Surface); defined manully
DEFINE_VULKAN_TYPE_INFO(Device);
DEFINE_VULKAN_TYPE_INFO(Image);
DEFINE_VULKAN_TYPE_INFO_KHR(Swapchain);
DEFINE_VULKAN_TYPE_INFO(RenderPass);

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

template <> struct VulkanTypeInfo<GLFWwindow *> : Destroyer<GLFWwindow *> {
  static constexpr auto Create = CreateGLFWwindow;
  static constexpr auto Destroy = DeleteGLFWwindow;
  static constexpr auto Name = "GLFWwindow";
};

template <> struct VulkanTypeInfo<VkSurfaceKHR> : Destroyer<VkSurfaceKHR> {
  static constexpr auto Destroy = vkDestroySurfaceKHR;
  static constexpr auto Create = glfwCreateWindowSurface;
  static constexpr auto Name = "VkSurfaceKHR";
};

} // namespace vkx

