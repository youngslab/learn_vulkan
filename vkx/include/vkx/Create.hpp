
#pragma once

#include <GLFW/glfw3.h>
#include <vkx/Object.hpp>
#include <vulkan/vulkan_core.h>

namespace vkx {

auto CreateInstance(const VkInstanceCreateInfo *pCreateInfo,
		    const VkAllocationCallbacks *pAllocator, Instance *pObject)
    -> VkResult;

auto CreateWindow(int width, int height, const char *title, Window *pWindow)
    -> VkResult;

auto CreateDebugReportCallbackEXT(
    Instance const &instance, const VkDebugReportCallbackCreateInfoEXT *pCreateInfo,
    const VkAllocationCallbacks *pAllocator, DebugReportCallbackEXT *pCallback)
    -> VkResult;

auto CreateSurfaceKHR(Instance const &instance, Window const &window,
		      const VkAllocationCallbacks *allocator,
		      SurfaceKHR *surface) -> VkResult;

auto CreateDevice(VkPhysicalDevice physicalDevice,
		  const VkDeviceCreateInfo *pCreateInfo,
		  const VkAllocationCallbacks *pAllocator, Device *pDevice)
    -> VkResult;

auto CreateSwapchainKHR(Device const &device,
			const VkSwapchainCreateInfoKHR *pCreateInfo,
			const VkAllocationCallbacks *pAllocator,
			SwapchainKHR *pSwapchain) -> VkResult;

auto CreateRenderPass(Device const &device, const VkRenderPassCreateInfo *pCreateInfo,
		      const VkAllocationCallbacks *pAllocator,
		      RenderPass *pRenderPass) -> VkResult;

auto CreateDescriptorSetLayout(
    Device const &device, const VkDescriptorSetLayoutCreateInfo *pCreateInfo,
    const VkAllocationCallbacks *pAllocator, DescriptorSetLayout *pSetLayout)
    -> VkResult;

auto CreatePipelineLayout(Device const &device,
			  const VkPipelineLayoutCreateInfo *pCreateInfo,
			  const VkAllocationCallbacks *pAllocator,
			  PipelineLayout *pPipelineLayout) -> VkResult;

auto CreateGraphicsPipelines(Device const &device, VkPipelineCache pipelineCache,
			     uint32_t createInfoCount,
			     const VkGraphicsPipelineCreateInfo *pCreateInfos,
			     const VkAllocationCallbacks *pAllocator,
			     Pipeline *pPipelines) -> VkResult;

auto CreateImage(Device const &device, const VkImageCreateInfo *pCreateInfo,
		 const VkAllocationCallbacks *pAllocator, Image *pImage)
    -> VkResult;

auto CreateImageView(Device const &device, const VkImageViewCreateInfo *pCreateInfo,
		     const VkAllocationCallbacks *pAllocator, ImageView *pView)
    -> VkResult;

auto CreateFramebuffer(Device const &device,
		       const VkFramebufferCreateInfo *pCreateInfo,
		       const VkAllocationCallbacks *pAllocator,
		       Framebuffer *pFramebuffer) -> VkResult;

auto CreateCommandPool(Device const &device,
		       const VkCommandPoolCreateInfo *pCreateInfo,
		       const VkAllocationCallbacks *pAllocator,
		       CommandPool *pCommandPool) -> VkResult;

auto CreateSampler(Device const &device, const VkSamplerCreateInfo *pCreateInfo,
		   const VkAllocationCallbacks *pAllocator, Sampler *pSampler)
    -> VkResult;

auto CreateBuffer(Device const &device, const VkBufferCreateInfo *pCreateInfo,
		  const VkAllocationCallbacks *pAllocator, Buffer *pBuffer)
    -> VkResult;

auto CreateDescriptorPool(Device const &device,
			  const VkDescriptorPoolCreateInfo *pCreateInfo,
			  const VkAllocationCallbacks *pAllocator,
			  DescriptorPool *pDescriptorPool) -> VkResult;

auto CreateSemaphore(Device const &device, const VkSemaphoreCreateInfo *pCreateInfo,
		     const VkAllocationCallbacks *pAllocator,
		     Semaphore *pSemaphore) -> VkResult;

auto CreateFence(Device const &device, const VkFenceCreateInfo *pCreateInfo,
		 const VkAllocationCallbacks *pAllocator, Fence *pFence)
    -> VkResult;

auto AllocateCommandBuffers(Device const &device, CommandPool const &commandPool,
			    const VkCommandBufferAllocateInfo *pAllocateInfo,
			    CommandBuffer *pCommandBuffers) -> VkResult;

auto AllocateMemory(Device const &device, const VkMemoryAllocateInfo *pAllocateInfo,
		    const VkAllocationCallbacks *pAllocator,
		    DeviceMemory *pMemory) -> VkResult;

auto AllocateDescriptorSets(Device const &device, DescriptorPool const &descriptorPool,
			    const VkDescriptorSetAllocateInfo *pAllocateInfo,
			    DescriptorSet *pDescriptorSets) -> VkResult;
} // namespace vkx
