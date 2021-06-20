#pragma once

#include <memory>
#include <functional>
#include <vkx/Resource.hpp>
#include <GLFW/glfw3.h>

namespace vkx {
template <typename T> class AutoDeletable {
private:
  std::shared_ptr<T> _storage;

public:
  AutoDeletable() : _storage(nullptr) {}
  AutoDeletable(T handle, std::function<void(T)> deleter)
      : _storage(std::shared_ptr<T>(new T(handle), [deleter](T *ptr) {
	  deleter(*ptr);
	  delete ptr;
	})) {}

  operator T() & { return *_storage; }
  operator T() && = delete;
  operator T() const & { return *_storage; }
  operator T() const && = delete;
  T *operator&() { return _storage.get(); }
};

template <typename T>
auto MakeAutoDeletable(T handle, std::function<void(T)> deleter)
    -> AutoDeletable<T> {
  return AutoDeletable(handle, deleter);
}

// top level
// handle과 deleter는 동시에 만들어 져야 한다.
// deleter는 create함수 인자와 다르기 때문에
// 인자를 잘 골라서 detele 쪽으로 넘겨 주어햐 한다.
// - Device : dependency를 걸어 주어야 한다.
// 아래 함수는 더이상 쪼갤 수 없다.
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

static auto CreateAutoDeletable(uint32_t w, uint32_t h, std::string title)
    -> AutoDeletable<GLFWwindow *> {
  auto handle = CreateResource<GLFWwindow *>(w, h, title);
  auto deleter = [](GLFWwindow *w) { DeleteResource<GLFWwindow *>(w); };
  return MakeAutoDeletable(handle, deleter);
}

} // namespace vkx
