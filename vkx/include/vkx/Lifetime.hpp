#pragma once

#include <memory>
#include <functional>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <vkx/TypeInfo.hpp>

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

  T *data() { return _storage.get(); }
};

template <typename T>
auto MakeAutoDeletable(T handle, std::function<void(T)> deleter)
    -> AutoDeletable<T> {
  return AutoDeletable(handle, deleter);
}

template <typename Resource, typename... Args>
auto CreateHandle(Args... args) -> Resource {
  Resource handle;
  auto result = VulkanTypeInfo<Resource>::Create(args..., &handle);
  if (result != VK_SUCCESS) {
    // TODO: formatting string with result.
    throw std::runtime_error(std::string("Failed to create a handle - ") +
			     VulkanTypeInfo<Resource>::Name);
  }
  return handle;
}

template <typename Resource, typename... Args>
auto CreateDeleter(Args... args) -> std::function<void(Resource)> {
  return [=](Resource handle) {
    VulkanTypeInfo<Resource>::Destroy2(handle, args...);
  };
}

} // namespace vkx
