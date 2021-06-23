#pragma once

#include <memory>
#include <functional>
#include <GLFW/glfw3.h>
#include <vkx/TypeInfo.hpp>
#include <vkx/Resource.hpp>

namespace vkx {
template <typename T> class AutoDeletable {
private:
  std::shared_ptr<T> _storage;

public:
  AutoDeletable() : _storage(nullptr) {}
	AutoDeletable(AutoDeletable const &rhs) : _storage(rhs._storage) {}
  AutoDeletable(T handle, std::function<void(T)> deleter)
      : _storage(std::shared_ptr<T>(new T(handle), [deleter](T *ptr) {
	  deleter(*ptr);
	  delete ptr;
	})) {}

  operator T() & { return *_storage; }
  operator T() && = delete;
  operator T() const & { return *_storage; }
  operator T() const && = delete;

  // T *operator&() { return _storage.get(); }
};

template <typename T>
auto MakeAutoDeletable(T handle, std::function<void(T)> deleter)
    -> AutoDeletable<T> {
  return AutoDeletable(handle, deleter);
}

template <typename Resource>
auto CreateAutoDeletable(
    VkDevice device,
    const typename VulkanTypeInfo<Resource>::CreateInfo *pCreateInfo,
    const VkAllocationCallbacks *pAllocator) -> AutoDeletable<Resource> {
  auto handle = CreateResource<Resource>(device, pCreateInfo, pAllocator);
  auto deleter = [&device, pAllocator](Resource handle) {
    DeleteResource<Resource>(device, handle, pAllocator);
  };
  return MakeAutoDeletable(handle, deleter);
}

template <typename T> auto CreateAutoDeletable() -> AutoDeletable<T> {
  return MakeAutoDeletable(VK_NULL_HANDLE, [](auto) {});
}

auto CreateAutoDeletable(uint32_t w, uint32_t h, std::string title)
    -> AutoDeletable<GLFWwindow *>;

auto CreateAutoDeletable(const VkInstanceCreateInfo *pCreateInfo,
			 const VkAllocationCallbacks *pAllocator)
    -> AutoDeletable<VkInstance>;

} // namespace vkx
