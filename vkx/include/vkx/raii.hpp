
#pragma once

#include <vulkan/vulkan.hpp>
#include <memory>
#include <GLFW/glfw3.h>
#include <vulkan/vulkan_core.h>

namespace vkx {

template <typename T> class Resource {
  // memory storage for a resource
  std::shared_ptr<T> _res;

public:
  // default constructor is not allowed
  // Resource() = delete;

  // member wise copy
  Resource() : _res(nullptr) {}
  Resource(std::shared_ptr<T> res) : _res(res) {}

  // Converter
  operator T() { return *_res; }
  operator T() const { return *_res; }
  operator T *() { return _res.get(); }
  operator T *() const { return _res.get(); }
};

class Instance : public Resource<VkInstance> {

private:
  using Resource::Resource;

public:
  static auto Create(const VkInstanceCreateInfo *pCreateInfo,
		     const VkAllocationCallbacks *pAllocator) -> Instance;
};

class DebugUtilsMessenger : public Resource<VkDebugUtilsMessengerEXT> {
private:
  using Resource::Resource;

public:
  static auto Create(Instance instance,
		     const VkDebugUtilsMessengerCreateInfoEXT *pCreateInfo,
		     const VkAllocationCallbacks *pAllocator)
      -> DebugUtilsMessenger;
};

class DebugReportCallback : public Resource<VkDebugReportCallbackEXT> {
private:
  using Resource::Resource;

public:
  static auto Create(Instance instance,
		     const VkDebugReportCallbackCreateInfoEXT *pCreateInfo,
		     const VkAllocationCallbacks *pAllocator)
      -> DebugReportCallback;
};

class Window : public Resource<GLFWwindow> {
private:
  using Resource::Resource;

public:
  static auto Create(uint32_t w, uint32_t h, std::string title) -> Window;
};

class Surface : public Resource<VkSurfaceKHR> {
private:
  using Resource::Resource;

public:
  static auto Create(Instance instance, Window window,
		     const VkAllocationCallbacks *allocator) -> Surface;
};

class Device : public Resource<VkDevice> {
private:
  using Resource::Resource;

public:
  static auto Create(VkPhysicalDevice const &physicalDevice,
		     const VkDeviceCreateInfo *pCreateInfo,
		     const VkAllocationCallbacks *pAllocator) -> Device;
};

class Swapchain : public Resource<VkSwapchainKHR> {
private:
  using Resource::Resource;
  VkFormat _format;
  VkExtent2D _extent;

public:
  static auto Create(Device const &device,
		     const VkSwapchainCreateInfoKHR *pCreateInfo,
		     const VkAllocationCallbacks *pAllocator) -> Swapchain;

  inline auto GetFormat() -> VkFormat { return _format; }
  inline auto GetExtent() -> VkExtent2D { return _extent; }
};

class ImageView : public Resource<VkImageView> {
private:
  using Resource::Resource;

public:
  static auto Create(Device const &device,
		     const VkImageViewCreateInfo *createInfo,
		     const VkAllocationCallbacks *pAllocator) -> ImageView;
};

class ShaderModule : public Resource<VkShaderModule> {

private:
  using Resource::Resource;

public:
  static auto Create(Device const &device,
		     const VkShaderModuleCreateInfo &createInfo,
		     const VkAllocationCallbacks *pAllocator) -> ShaderModule;
};

} // namespace vkx
