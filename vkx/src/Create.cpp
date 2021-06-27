
#include "vkx/Object.hpp"
#include <vulkan/vulkan_core.h>

namespace vkx {

auto CreateInstance(const VkInstanceCreateInfo *pCreateInfo,
		    const VkAllocationCallbacks *pAllocator, Instance *pObject)
    -> VkResult {
  return CreateObject<Instance>(pCreateInfo, pAllocator, pObject);
  // return CreateObject2<VkInstance>(pCreateInfo, pAllocator, pObject);
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
  // return CreateObject2<GLFWwindow*>(width, height, title, pWindow);
}

auto CreateDebugReportCallbackEXT(
    Instance instance, const VkDebugReportCallbackCreateInfoEXT *pCreateInfo,
    const VkAllocationCallbacks *pAllocator, DebugReportCallbackEXT *pCallback)
    -> VkResult {
  // return CreateObject2<VkDebugReportCallbackEXT>(instance, pCreateInfo,
  // pAllocator, pCallback);
  return CreateObject<DebugReportCallbackEXT>(instance, pCreateInfo, pAllocator,
					      pCallback);
}

auto CreateSurfaceKHR(Instance instance, Window window,
		      const VkAllocationCallbacks *allocator,
		      SurfaceKHR *surface) -> VkResult {
  return CreateObject<SurfaceKHR>(instance, window, allocator, surface);
  // return CreateObject2<VkSurfaceKHR>(instance, window, allocator, surface);
}

auto CreateDevice(VkPhysicalDevice physicalDevice,
		  const VkDeviceCreateInfo *pCreateInfo,
		  const VkAllocationCallbacks *pAllocator, Device *pDevice)
    -> VkResult {
  return CreateObject<Device>(physicalDevice, pCreateInfo, pAllocator, pDevice);
}

auto CreateSwapchainKHR(Device device,
			const VkSwapchainCreateInfoKHR *pCreateInfo,
			const VkAllocationCallbacks *pAllocator,
			SwapchainKHR *pSwapchain) -> VkResult {
  return CreateObject<SwapchainKHR>(device, pCreateInfo, pAllocator,
				    pSwapchain);
}

auto CreateRenderPass(Device device, const VkRenderPassCreateInfo *pCreateInfo,
		      const VkAllocationCallbacks *pAllocator,
		      RenderPass *pRenderPass) -> VkResult {
  return CreateObject<RenderPass>(device, pCreateInfo, pAllocator, pRenderPass);
}

auto CreateDescriptorSetLayout(
    Device device, const VkDescriptorSetLayoutCreateInfo *pCreateInfo,
    const VkAllocationCallbacks *pAllocator, DescriptorSetLayout *pSetLayout)
    -> VkResult {
  return CreateObject<DescriptorSetLayout>(device, pCreateInfo, pAllocator,
					   pSetLayout);
}

auto CreatePipelineLayout(Device device,
			  const VkPipelineLayoutCreateInfo *pCreateInfo,
			  const VkAllocationCallbacks *pAllocator,
			  PipelineLayout *pPipelineLayout) -> VkResult {
  return CreateObject<PipelineLayout>(device, pCreateInfo, pAllocator,
				      pPipelineLayout);
}

auto CreateGraphicsPipelines(Device device, VkPipelineCache pipelineCache,
			     uint32_t createInfoCount,
			     const VkGraphicsPipelineCreateInfo *pCreateInfos,
			     const VkAllocationCallbacks *pAllocator,
			     Pipeline *pPipelines) -> VkResult {
  // TODO: Supprots creating multiple instnace.
  return CreateObject<Pipeline>(device, pipelineCache, createInfoCount,
				pCreateInfos, pAllocator, pPipelines);
}

auto CreateImage(Device device, const VkImageCreateInfo *pCreateInfo,
		 const VkAllocationCallbacks *pAllocator, Image *pImage)
    -> VkResult {
  return CreateObject<Image>(device, pCreateInfo, pAllocator, pImage);
}

} // namespace vkx
