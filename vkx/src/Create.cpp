
#include "vkx/Object.hpp"

namespace vkx {

auto CreateInstance(const VkInstanceCreateInfo *pCreateInfo,
		    const VkAllocationCallbacks *pAllocator, Instance *pObject)
    -> VkResult {
  return CreateObject(pCreateInfo, pAllocator, pObject);
}

auto CreateInstance(const VkInstanceCreateInfo *pCreateInfo,
		    const VkAllocationCallbacks *pAllocator) -> Instance {
  Instance handle;
  auto res = CreateInstance(pCreateInfo, pAllocator, &handle);
  if (res != VK_SUCCESS) {
    throw std::runtime_error("failed to create instance");
  }
  return handle;
}

auto CreateWindow(int width, int height, const char *title, Window *pWindow)
    -> VkResult {
  return CreateObject(width, height, title, pWindow);
}

auto CreateDebugReportCallbackEXT(
    Instance instance, const VkDebugReportCallbackCreateInfoEXT *pCreateInfo,
    const VkAllocationCallbacks *pAllocator, DebugReportCallbackEXT *pCallback)
    -> VkResult {
  return CreateObject(instance, pCreateInfo, pAllocator, pCallback);
}

} // namespace vkx
