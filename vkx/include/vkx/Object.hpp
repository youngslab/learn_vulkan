
#pragma once

#include <memory>
#include <functional>
#include <vkx/AutoDeletable.hpp>
#include <vkx/TypeInfo.hpp>
#include <vkx/Tmp.hpp>
#include <vkx/Dependable.hpp>

namespace vkx {

// manage dependency
template <typename Resource>
class Object : public Dependable, public AutoDeletable<Resource> {
private:
  template <typename T> void DependIf(Object<T> object) {
    this->Depend(object);
  }
  template <typename T> void DependIf(T t) {}

public:
  using ResouceType = Resource;

  Object() {}

  Object(Object const &rhs) : AutoDeletable<Resource>(rhs), Dependable(rhs) {}

  Object(std::function<void(Resource)> deleter)
      : AutoDeletable<Resource>(VK_NULL_HANDLE, deleter) {}

  Object(Resource handle, std::function<void(Resource)> deleter)
      : AutoDeletable<Resource>(handle, deleter) {}

  template <typename... Args>
  Object(Args... args)
      : AutoDeletable<Resource>(CreateHandle<Resource>(args...),
				CreateDeleterDebug<Resource>(args...)) {
    bool z[] = {(this->DependIf(args), true)...};
    (void)z;
  }
};

// Abstract the way to create vulkan objects.
template <typename ObjectType, typename... Args> //
auto CreateObject(Args &&...args) -> VkResult {
  ObjectType *pObject = get_last(args...);
  try {
    *pObject = std::apply(make_object<ObjectType>{}, drop_last(args...));
    return VK_SUCCESS;
  } catch (...) {
    return VK_ERROR_UNKNOWN;
  }
}

using Instance = Object<VkInstance>;
using Window = Object<GLFWwindow *>;
using DebugReportCallbackEXT = Object<VkDebugReportCallbackEXT>;
using SurfaceKHR = Object<VkSurfaceKHR>;
using Device = Object<VkDevice>;
using SwapchainKHR = Object<VkSwapchainKHR>;
using RenderPass = Object<VkRenderPass>;
using DescriptorSetLayout = Object<VkDescriptorSetLayout>;
using PipelineLayout = Object<VkPipelineLayout>;
using Pipeline = Object<VkPipeline>;
using Image = Object<VkImage>;
using ImageView = Object<VkImageView>;
using Framebuffer = Object<VkFramebuffer>;
using CommandPool = Object<VkCommandPool>;
using CommandBuffer = Object<VkCommandBuffer>;
using Sampler = Object<VkSampler>;
using Buffer = Object<VkBuffer>;
using DeviceMemory = Object<VkDeviceMemory>;
using DescriptorPool = Object<VkDescriptorPool>;
using DescriptorSet = Object<VkDescriptorSet>;
using Semaphore = Object<VkSemaphore>;
using Fence = Object<VkFence>;

} // namespace vkx
