
#pragma once

#include <memory>
#include <functional>
#include <vkx/Lifetime.hpp>
#include <vkx/Tmp.hpp>

namespace vkx {

// manage dependency
template <typename Resource> class Object : public AutoDeletable<Resource> {
public:
  Object() {}
  Object(Object const &rhs) : AutoDeletable<Resource>(rhs) {}
  Object(Resource handle, std::function<void(Resource)> deleter)
      : AutoDeletable<Resource>(handle, deleter) {}

  template <typename... Args>
  Object(Args... args)
      : AutoDeletable<Resource>(CreateHandle<Resource>(args...),
				CreateDeleter<Resource>(args...)) {}
};

// Abstract the way to create vulkan objects.
template <typename... Args> //
auto CreateObject(Args... args) -> VkResult {
  using ObjectType = std::remove_pointer_t<last_t<Args...>>;
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
