
#pragma once

#include <iostream>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <vulkan/vulkan.hpp>
#include <vulkan/vulkan_core.h>
#include <vkx/Ext.hpp>

namespace vkx {

template <typename T> struct VulkanTypeInfo;

#define DEFINE_VULKAN_TYPE_INFO(RESOURCE)                                      \
  template <> struct VulkanTypeInfo<Vk##RESOURCE> {                            \
    static constexpr auto Create = vkCreate##RESOURCE;                         \
    static constexpr auto Destroy = vkDestroy##RESOURCE;                       \
    static constexpr auto Name = "Vk" #RESOURCE;                               \
  };

#define DEFINE_VULKAN_TYPE_INFO_KHR(RESOURCE)                                  \
  template <> struct VulkanTypeInfo<Vk##RESOURCE##KHR> {                       \
    static constexpr auto Create = vkCreate##RESOURCE##KHR;                    \
    static constexpr auto Destroy = vkDestroy##RESOURCE##KHR;                  \
    static constexpr auto Name = "Vk" #RESOURCE;                               \
  };

#define DEFINE_VULKAN_TYPE_INFO_EXT(RESOURCE)                                  \
  template <> struct VulkanTypeInfo<Vk##RESOURCE##EXT> {                       \
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
DEFINE_VULKAN_TYPE_INFO(ImageView);
DEFINE_VULKAN_TYPE_INFO_KHR(Swapchain);
DEFINE_VULKAN_TYPE_INFO(RenderPass);
DEFINE_VULKAN_TYPE_INFO(DescriptorSetLayout);
DEFINE_VULKAN_TYPE_INFO(PipelineLayout);
DEFINE_VULKAN_TYPE_INFO(Framebuffer);
DEFINE_VULKAN_TYPE_INFO(CommandPool);
DEFINE_VULKAN_TYPE_INFO(Sampler);
DEFINE_VULKAN_TYPE_INFO(Buffer);
DEFINE_VULKAN_TYPE_INFO(DescriptorPool);

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

template <> struct VulkanTypeInfo<VkSurfaceKHR> {
  static constexpr auto Destroy = vkDestroySurfaceKHR;
  static constexpr auto Create = glfwCreateWindowSurface;
  static constexpr auto Name = "VkSurfaceKHR";
};

template <> struct VulkanTypeInfo<VkDeviceMemory> {
  static constexpr auto Destroy = vkFreeMemory;
  static constexpr auto Create = vkAllocateMemory;
  static constexpr auto Name = "VkDeviceMemory";
};

// special case
template <> struct VulkanTypeInfo<VkPipeline> {
  static constexpr auto Name = "VkPipeline";
  static constexpr auto Destroy = vkDestroyPipeline;
  static auto Create(VkDevice device, VkPipelineCache pipelineCache,
		     uint32_t createInfoCount,
		     const VkGraphicsPipelineCreateInfo *pCreateInfos,
		     const VkAllocationCallbacks *pAllocator,
		     VkPipeline *pPipelines) {
    return vkCreateGraphicsPipelines(device, pipelineCache, createInfoCount,
				     pCreateInfos, pAllocator, pPipelines);
  }

  static auto Create(VkDevice device, VkPipelineCache pipelineCache,
		     uint32_t createInfoCount,
		     const VkComputePipelineCreateInfo *pCreateInfos,
		     const VkAllocationCallbacks *pAllocator,
		     VkPipeline *pPipelines) {
    return vkCreateComputePipelines(device, pipelineCache, createInfoCount,
				    pCreateInfos, pAllocator, pPipelines);
  }
};

static auto
AllocateCommandBuffers(VkDevice device, VkCommandPool,
		       const VkCommandBufferAllocateInfo *pAllocateInfo,
		       VkCommandBuffer *pCommandBuffers) {
  return vkAllocateCommandBuffers(device, pAllocateInfo, pCommandBuffers);
}

template <> struct VulkanTypeInfo<VkCommandBuffer> {
  static constexpr auto Name = "VkCommandBuffer";
  static constexpr auto Destroy = vkFreeCommandBuffers;
  static constexpr auto Create = AllocateCommandBuffers;
};

template <typename Resource, typename... Args>
auto CreateHandle(Args... args) -> Resource {
  Resource handle;
  auto result = VulkanTypeInfo<Resource>::Create(args..., &handle);
  if (result != VK_SUCCESS) {
    // TODO: formatting string with result.
    throw std::runtime_error(std::string("Failed to create a handle - ") +
			     VulkanTypeInfo<Resource>::Name);
  }
  return handle;
}

template <typename T, typename Dependency, typename CreateInfo>
static auto CreateDeleter(Dependency dep, CreateInfo,
			  const VkAllocationCallbacks *pAllocator)
    -> std::function<void(T)> {
  return [dep, pAllocator](T handle) {
    VulkanTypeInfo<T>::Destroy(dep, handle, pAllocator);
  };
}

template <typename T, typename CreateInfo>
static auto CreateDeleter(const CreateInfo *,
			  const VkAllocationCallbacks *pAllocator)
    -> std::function<void(T)> {
  return [pAllocator](T handle) {
    VulkanTypeInfo<T>::Destroy(handle, pAllocator);
  };
}

template <typename T>
static auto CreateDeleter(int, int, const char *) -> std::function<void(T)> {
  return [](T handle) { VulkanTypeInfo<T>::Destroy(handle); };
}

template <typename T>
static auto CreateDeleter(VkPhysicalDevice, const VkDeviceCreateInfo *,
			  const VkAllocationCallbacks *pAllocator)
    -> std::function<void(T)> {
  return [pAllocator](T handle) {
    VulkanTypeInfo<T>::Destroy(handle, pAllocator);
  };
}

template <typename T, typename CreateInfo>
static auto CreateDeleter(VkDevice device, VkPipelineCache pipelineCache,
			  uint32_t createInfoCount,
			  const CreateInfo *pCreateInfos,
			  const VkAllocationCallbacks *pAllocator)
    -> std::function<void(T)> {
  return [device, pAllocator](T handle) {
    VulkanTypeInfo<T>::Destroy(device, handle, pAllocator);
  };
}

template <typename T>
static auto CreateDeleter(VkDevice device, VkCommandPool commandPool,
			  const VkCommandBufferAllocateInfo *pAllocateInfo)
    -> std::function<void(T)> {
  return [device, commandPool](T handle) {
    VulkanTypeInfo<T>::Destroy(device, commandPool, 1, &handle);
  };
}

template <typename T, typename... Args>
static auto CreateDeleterDebug(Args... args) -> std::function<void(T)> {
  auto deleter = CreateDeleter<T>(args...);
  return [deleter](T handle) {
    std::cout << "delete " << VulkanTypeInfo<T>::Name << "\n";
    deleter(handle);
  };
}

} // namespace vkx

