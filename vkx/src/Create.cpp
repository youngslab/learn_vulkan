
#include "vkx/Object.hpp"

namespace vkx {

auto CreateInstance(const VkInstanceCreateInfo *pCreateInfo,
		    const VkAllocationCallbacks *pAllocator, Instance *pObject)
    -> VkResult {
	return CreateObject<Instance>(pCreateInfo, pAllocator, pObject);
	//return CreateObject2<VkInstance>(pCreateInfo, pAllocator, pObject);
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
	return CreateObject<Window>(width, height, title, pWindow);
  //return CreateObject2<GLFWwindow*>(width, height, title, pWindow);
}

auto CreateDebugReportCallbackEXT(
    Instance instance, const VkDebugReportCallbackCreateInfoEXT *pCreateInfo,
    const VkAllocationCallbacks *pAllocator, DebugReportCallbackEXT *pCallback)
    -> VkResult {
  //return CreateObject2<VkDebugReportCallbackEXT>(instance, pCreateInfo, pAllocator, pCallback);
	return CreateObject<DebugReportCallbackEXT>(instance, pCreateInfo, pAllocator, pCallback);
}

auto CreateSurfaceKHR(Instance instance, Window window,
		      const VkAllocationCallbacks *allocator,
		      SurfaceKHR *surface) -> VkResult {
	return CreateObject<SurfaceKHR>(instance, window, allocator, surface);
  //return CreateObject2<VkSurfaceKHR>(instance, window, allocator, surface);
}

} // namespace vkx
