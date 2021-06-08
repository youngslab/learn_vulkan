
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
  operator T() & { return *_res; }
  operator T() && = delete;
  operator T() const & { return *_res; }
  operator T() const && = delete;
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

  inline auto GetFormat() const -> VkFormat { return _format; }
  inline auto GetExtent() const -> VkExtent2D { return _extent; }
};

class ImageView : public Resource<VkImageView> {
private:
  using Resource::Resource;

public:
  static auto Create(Device const &device,
		     const VkImageViewCreateInfo *createInfo,
		     const VkAllocationCallbacks *pAllocator) -> ImageView;
};

class RenderPass : public Resource<VkRenderPass> {
private:
  using Resource::Resource;

public:
  static auto Create(Device const &device,
		     const VkRenderPassCreateInfo *pCreateInfo,
		     const VkAllocationCallbacks *pAllocator) -> RenderPass;
};

class ShaderModule : public Resource<VkShaderModule> {

private:
  using Resource::Resource;

public:
  static auto Create(Device const &device,
		     const VkShaderModuleCreateInfo *pCreateInfo,
		     const VkAllocationCallbacks *pAllocator) -> ShaderModule;
};

inline auto CreateShaderModule(Device const &device,
			       const VkShaderModuleCreateInfo *pCreateInfo,
			       const VkAllocationCallbacks *pAllocator)
    -> ShaderModule {
  return ShaderModule::Create(device, pCreateInfo, pAllocator);
}

class DescriptorSetLayout : public Resource<VkDescriptorSetLayout> {
private:
  using Resource::Resource;

public:
  static auto Create(Device const &device,
		     const VkDescriptorSetLayoutCreateInfo *pCreateInfo,
		     const VkAllocationCallbacks *pAllocator)
      -> DescriptorSetLayout;
};

inline auto CreateDescriptorSetLayout(
    Device const &device, const VkDescriptorSetLayoutCreateInfo *pCreateInfo,
    const VkAllocationCallbacks *pAllocator) -> DescriptorSetLayout {
  return DescriptorSetLayout::Create(device, pCreateInfo, pAllocator);
}

class DescriptorPool : public Resource<VkDescriptorPool> {
private:
  using Resource::Resource;

public:
  static auto Create(Device const &device,
		     const VkDescriptorPoolCreateInfo *pCreateInfo,
		     const VkAllocationCallbacks *pAllocator) -> DescriptorPool;
};

inline auto CreateDescriptorPool(Device const &device,
				 const VkDescriptorPoolCreateInfo *pCreateInfo,
				 const VkAllocationCallbacks *pAllocator)
    -> DescriptorPool {
  return DescriptorPool::Create(device, pCreateInfo, pAllocator);
}

class PipelineLayout : public Resource<VkPipelineLayout> {
private:
  using Resource::Resource;

public:
  static auto Create(Device const &device,
		     const VkPipelineLayoutCreateInfo *pCreateInfo,
		     const VkAllocationCallbacks *pAllocator) -> PipelineLayout;
};

inline auto CreatePipelineLayout(Device const &device,
				 const VkPipelineLayoutCreateInfo *pCreateInfo,
				 const VkAllocationCallbacks *pAllocator)
    -> PipelineLayout {
  return PipelineLayout::Create(device, pCreateInfo, pAllocator);
}

class Pipeline : public Resource<VkPipeline> {
private:
  using Resource::Resource;

public:
  static auto Create(Device const &device, VkPipelineCache pipelineCache,
		     const VkGraphicsPipelineCreateInfo *pCreateInfo,
		     const VkAllocationCallbacks *pAllocator) -> Pipeline;
};

inline auto CreatePipeline(Device const &device, VkPipelineCache pipelineCache,
			   const VkGraphicsPipelineCreateInfo *pCreateInfo,
			   const VkAllocationCallbacks *pAllocator)
    -> Pipeline {
  return Pipeline::Create(device, pipelineCache, pCreateInfo, pAllocator);
}

} // namespace vkx
