
#pragma once

#include <vkx/TypeInfo.hpp>

namespace vkx {
// common way to create a vulkan resource
template <typename T, typename... Args> //
inline auto CreateResource(Args... args) -> T {
  T handle;
  auto result = VulkanTypeInfo<T>::Create(args..., &handle);
  if (result != VK_SUCCESS) {
    // TODO: formatting string with result.
    throw std::runtime_error(std::string("Failed to create a handle - ") +
			     VulkanTypeInfo<T>::Name);
  }
  return handle;
}

template <typename Resource, typename... Args>
inline auto DeleteResource(Args &&...args) -> void {
  VulkanTypeInfo<Resource>::Destroy(args...);
}

} // namespace vkx
