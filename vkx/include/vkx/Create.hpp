
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

} // namespace vkx
