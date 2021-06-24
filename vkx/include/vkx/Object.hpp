
#pragma once

#include <memory>
#include <functional>
#include <vkx/Lifetime.hpp>
#include <vkx/Tmp.hpp>
#include <vkx/Dependable.hpp>

namespace vkx {

// manage dependency
template <typename Resource>
class Object : public AutoDeletable<Resource>, public Dependable {
public:
  Object() {}
  Object(Object const &rhs) : AutoDeletable<Resource>(rhs), Dependable(rhs) {}
  Object(std::function<void(Resource)> deleter)
      : AutoDeletable<Resource>(VK_NULL_HANDLE, deleter) {}
  Object(Resource handle, std::function<void(Resource)> deleter)
      : AutoDeletable<Resource>(handle, deleter) {}

  template <typename T, typename... Args>
  Object(Object<T> dependency, Args... args)
      : AutoDeletable<Resource>(CreateHandle<Resource>(dependency, args...),
				CreateDeleter<Resource>(dependency, args...)) {
    this->Depend(dependency);
  }

  template <typename... Args>
  Object(Args... args)
      : AutoDeletable<Resource>(CreateHandle<Resource>(args...),
				CreateDeleter<Resource>(args...)) {}
};

// template <typename Resource, typename... Args> //
// auto CreateObject(Args... args) -> VkResult {
// using ObjectType = std::remove_pointer_t<last_t<Args...>>;
// auto deleter = std::apply(VulkanTypeInfo<Resource>::Destroy2,
// drop_last(args...));
// ObjectType *pObject = get_last(args...);
//*pObject = Object(deleter);
// return  std::apply(VulkanTypeInfo<Resource>::Create,
// std::tuple_cat(drop_last(args...), pObject->data()));
//}

// Abstract the way to create vulkan objects.
template <typename ObjectType, typename... Args> //
auto CreateObject(Args... args) -> VkResult {
  ObjectType *pObject = get_last(args...);
  try {
    *pObject = std::apply(make_object<ObjectType>{}, drop_last(args...));
    return VK_SUCCESS;
  } catch (...) {
    return VK_ERROR_UNKNOWN;
  }
}

using Instance = Object<VkInstance>;
using Window = Object<GLFWwindow *>;
using DebugReportCallbackEXT = Object<VkDebugReportCallbackEXT>;
using SurfaceKHR = Object<VkSurfaceKHR>;

} // namespace vkx
