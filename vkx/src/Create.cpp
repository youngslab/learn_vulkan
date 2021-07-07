
#include "vkx/Object.hpp"
#include <vulkan/vulkan_core.h>

namespace vkx {

auto CreateInstance(const VkInstanceCreateInfo *pCreateInfo,
		    const VkAllocationCallbacks *pAllocator, Instance *pObject)
    -> VkResult {
  return CreateObject<Instance>(pCreateInfo, pAllocator, pObject);
}

auto CreateWindow(int width, int height, const char *title, Window *pWindow)
    -> VkResult {
  return CreateObject<Window>(width, height, title, pWindow);
}

auto CreateDebugReportCallbackEXT(
    Instance const &instance,
    const VkDebugReportCallbackCreateInfoEXT *pCreateInfo,
    const VkAllocationCallbacks *pAllocator, DebugReportCallbackEXT *pCallback)
    -> VkResult {
  return CreateObject<DebugReportCallbackEXT>(instance, pCreateInfo, pAllocator,
					      pCallback);
}

auto CreateSurfaceKHR(Instance const &instance, Window const &window,
		      const VkAllocationCallbacks *allocator,
		      SurfaceKHR *surface) -> VkResult {
  return CreateObject<SurfaceKHR>(instance, window, allocator, surface);
}

auto CreateDevice(VkPhysicalDevice physicalDevice,
		  const VkDeviceCreateInfo *pCreateInfo,
		  const VkAllocationCallbacks *pAllocator, Device *pDevice)
    -> VkResult {
  return CreateObject<Device>(physicalDevice, pCreateInfo, pAllocator, pDevice);
}

auto CreateSwapchainKHR(Device const &device,
			const VkSwapchainCreateInfoKHR *pCreateInfo,
			const VkAllocationCallbacks *pAllocator,
			SwapchainKHR *pSwapchain) -> VkResult {
  return CreateObject<SwapchainKHR>(device, pCreateInfo, pAllocator,
				    pSwapchain);
}

auto CreateRenderPass(Device const &device,
		      const VkRenderPassCreateInfo *pCreateInfo,
		      const VkAllocationCallbacks *pAllocator,
		      RenderPass *pRenderPass) -> VkResult {
  return CreateObject<RenderPass>(device, pCreateInfo, pAllocator, pRenderPass);
}

auto CreateDescriptorSetLayout(
    Device const &device, const VkDescriptorSetLayoutCreateInfo *pCreateInfo,
    const VkAllocationCallbacks *pAllocator, DescriptorSetLayout *pSetLayout)
    -> VkResult {
  return CreateObject<DescriptorSetLayout>(device, pCreateInfo, pAllocator,
					   pSetLayout);
}

auto CreatePipelineLayout(Device const &device,
			  const VkPipelineLayoutCreateInfo *pCreateInfo,
			  const VkAllocationCallbacks *pAllocator,
			  PipelineLayout *pPipelineLayout) -> VkResult {
  return CreateObject<PipelineLayout>(device, pCreateInfo, pAllocator,
				      pPipelineLayout);
}

auto CreateGraphicsPipelines(Device const &device,
			     VkPipelineCache pipelineCache,
			     uint32_t createInfoCount,
			     const VkGraphicsPipelineCreateInfo *pCreateInfos,
			     const VkAllocationCallbacks *pAllocator,
			     Pipeline *pPipelines) -> VkResult {
  auto res = VK_SUCCESS;
  for (int i = 0; i < createInfoCount; i++) {
    auto res = CreateObject<Pipeline>(
	device, pipelineCache, 1, &pCreateInfos[i], pAllocator, &pPipelines[i]);
    if (res != VK_SUCCESS)
      return res;
  }
  return res;
}

auto CreateImage(Device const &device, const VkImageCreateInfo *pCreateInfo,
		 const VkAllocationCallbacks *pAllocator, Image *pImage)
    -> VkResult {
  return CreateObject<Image>(device, pCreateInfo, pAllocator, pImage);
}

auto CreateImageView(Device const &device,
		     const VkImageViewCreateInfo *pCreateInfo,
		     const VkAllocationCallbacks *pAllocator, ImageView *pView)
    -> VkResult {
  return CreateObject<ImageView>(device, pCreateInfo, pAllocator, pView);
}

auto CreateFramebuffer(Device const &device,
		       const VkFramebufferCreateInfo *pCreateInfo,
		       const VkAllocationCallbacks *pAllocator,
		       Framebuffer *pFramebuffer) -> VkResult {
  return CreateObject<Framebuffer>(device, pCreateInfo, pAllocator,
				   pFramebuffer);
}

auto CreateCommandPool(Device const &device,
		       const VkCommandPoolCreateInfo *pCreateInfo,
		       const VkAllocationCallbacks *pAllocator,
		       CommandPool *pCommandPool) -> VkResult {
  return CreateObject<CommandPool>(device, pCreateInfo, pAllocator,
				   pCommandPool);
}

auto CreateSampler(Device const &device, const VkSamplerCreateInfo *pCreateInfo,
		   const VkAllocationCallbacks *pAllocator, Sampler *pSampler)
    -> VkResult {
  return CreateObject<Sampler>(device, pCreateInfo, pAllocator, pSampler);
}

auto CreateBuffer(Device const &device, const VkBufferCreateInfo *pCreateInfo,
		  const VkAllocationCallbacks *pAllocator, Buffer *pBuffer)
    -> VkResult {
  return CreateObject<Buffer>(device, pCreateInfo, pAllocator, pBuffer);
}

auto CreateDescriptorPool(Device const &device,
			  const VkDescriptorPoolCreateInfo *pCreateInfo,
			  const VkAllocationCallbacks *pAllocator,
			  DescriptorPool *pDescriptorPool) -> VkResult {
  return CreateObject<DescriptorPool>(device, pCreateInfo, pAllocator,
				      pDescriptorPool);
}

auto CreateSemaphore(Device const &device,
		     const VkSemaphoreCreateInfo *pCreateInfo,
		     const VkAllocationCallbacks *pAllocator,
		     Semaphore *pSemaphore) -> VkResult {
  return CreateObject<Semaphore>(device, pCreateInfo, pAllocator, pSemaphore);
}

auto CreateFence(Device const &device, const VkFenceCreateInfo *pCreateInfo,
		 const VkAllocationCallbacks *pAllocator, Fence *pFence)
    -> VkResult {
  return CreateObject<Fence>(device, pCreateInfo, pAllocator, pFence);
}

auto AllocateCommandBuffers(Device const &device,
			    CommandPool const &commandPool,
			    const VkCommandBufferAllocateInfo *pAllocateInfo,
			    CommandBuffer *pCommandBuffers) -> VkResult {
  auto count = pAllocateInfo->commandBufferCount;
  auto res = VK_SUCCESS;
  auto info = *pAllocateInfo;
  info.commandBufferCount = 1;
  for (auto i = 0u; i < count; i++) {
    res = CreateObject<CommandBuffer>(device, commandPool, &info,
				      &pCommandBuffers[i]);
    if (res != VK_SUCCESS)
      return res;
  }
  return res;
}

auto AllocateMemory(Device const &device,
		    const VkMemoryAllocateInfo *pAllocateInfo,
		    const VkAllocationCallbacks *pAllocator,
		    DeviceMemory *pMemory) -> VkResult {
  return CreateObject<DeviceMemory>(device, pAllocateInfo, pAllocator, pMemory);
}

auto AllocateDescriptorSets(Device const &device,
			    DescriptorPool const &descriptorPool,
			    const VkDescriptorSetAllocateInfo *pAllocateInfo,
			    DescriptorSet *pDescriptorSets) -> VkResult {

  auto count = pAllocateInfo->descriptorSetCount;
  auto res = VK_SUCCESS;
  auto info = *pAllocateInfo;
  info.descriptorSetCount = 1;
  for (auto i = 0u; i < count; i++) {
    res = CreateObject<DescriptorSet>(device, descriptorPool, &info,
				      &pDescriptorSets[i]);
    if (res != VK_SUCCESS)
      return res;
  }
  return res;
}

template <typename T> auto ConvertNative(std::vector<T> const &objects) {
  return std::transform(objects.begin(), objects.end(),
			[](T const &object) { return object.GetValue(); });
}

} // namespace vkx
