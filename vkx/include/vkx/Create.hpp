
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
    Instance instance, const VkDebugReportCallbackCreateInfoEXT *pCreateInfo,
    const VkAllocationCallbacks *pAllocator, DebugReportCallbackEXT *pCallback)
    -> VkResult;

auto CreateSurfaceKHR(Instance instance, Window window,
		      const VkAllocationCallbacks *allocator,
		      SurfaceKHR *surface) -> VkResult;

auto CreateDevice(VkPhysicalDevice physicalDevice,
		  const VkDeviceCreateInfo *pCreateInfo,
		  const VkAllocationCallbacks *pAllocator, Device *pDevice)
    -> VkResult;

auto CreateSwapchainKHR(Device device,
			const VkSwapchainCreateInfoKHR *pCreateInfo,
			const VkAllocationCallbacks *pAllocator,
			SwapchainKHR *pSwapchain) -> VkResult;

auto CreateRenderPass(Device device, const VkRenderPassCreateInfo *pCreateInfo,
		      const VkAllocationCallbacks *pAllocator,
		      RenderPass *pRenderPass) -> VkResult;

auto CreateDescriptorSetLayout(
    Device device, const VkDescriptorSetLayoutCreateInfo *pCreateInfo,
    const VkAllocationCallbacks *pAllocator, DescriptorSetLayout *pSetLayout)
    -> VkResult;

auto CreatePipelineLayout(Device device,
			  const VkPipelineLayoutCreateInfo *pCreateInfo,
			  const VkAllocationCallbacks *pAllocator,
			  PipelineLayout *pPipelineLayout) -> VkResult;

auto CreateGraphicsPipelines(Device device, VkPipelineCache pipelineCache,
			     uint32_t createInfoCount,
			     const VkGraphicsPipelineCreateInfo *pCreateInfos,
			     const VkAllocationCallbacks *pAllocator,
			     Pipeline *pPipelines) -> VkResult;

auto CreateImage(Device device, const VkImageCreateInfo *pCreateInfo,
		 const VkAllocationCallbacks *pAllocator, Image *pImage)
    -> VkResult;

auto CreateImageView(Device device, const VkImageViewCreateInfo *pCreateInfo,
		     const VkAllocationCallbacks *pAllocator, ImageView *pView)
    -> VkResult;

auto CreateFramebuffer(Device device,
		       const VkFramebufferCreateInfo *pCreateInfo,
		       const VkAllocationCallbacks *pAllocator,
		       Framebuffer *pFramebuffer) -> VkResult;

auto CreateCommandPool(Device device,
		       const VkCommandPoolCreateInfo *pCreateInfo,
		       const VkAllocationCallbacks *pAllocator,
		       CommandPool *pCommandPool) -> VkResult;

auto CreateSampler(Device device, const VkSamplerCreateInfo *pCreateInfo,
		   const VkAllocationCallbacks *pAllocator, Sampler *pSampler)
    -> VkResult;

auto AllocateCommandBuffers(Device device, CommandPool commandPool,
			    const VkCommandBufferAllocateInfo *pAllocateInfo,
			    CommandBuffer *pCommandBuffers) -> VkResult;

} // namespace vkx
