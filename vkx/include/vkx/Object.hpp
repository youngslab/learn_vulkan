
#pragma once

#include <memory>
#include <functional>
#include <vkx/Lifetime.hpp>
#include <tuple>

namespace vkx {

// manage dependency
template <typename Resource> class Object : public AutoDeletable<Resource> {
public:
  Object() {}

  template <typename... Args>
  Object(Args &&...args)
      : AutoDeletable<Resource>(CreateAutoDeletable(args...)) {}
};

template <typename Tuple, std::size_t... Ints>
auto select(Tuple &&tuple, std::index_sequence<Ints...>) {
  return std::tuple{std::get<Ints>(std::forward<Tuple>(tuple))...};
}

template <template <typename...> class Tuple, typename... Args>
auto tail(Tuple<Args...> const &tuple) {
  return std::get<sizeof...(Args) - 1>(tuple);
}

template <template <typename...> class Tuple, typename... Args>
auto drop_last(Tuple<Args...> const &tuple) {
  return select(tuple, std::make_index_sequence<sizeof...(Args) - 1>{});
}

template <typename T> struct make_object {
  template <typename... Args> auto operator()(Args... args) -> T {
    return T(args...);
  }
};

using Instance = Object<VkInstance>;

template <typename... Args> //
auto Create(Args... args) -> VkResult {
  using LastType = decltype(tail(std::tuple{args...}));
  using ObjectType = std::remove_pointer_t<LastType>;
  ObjectType *last = tail(std::tuple(args...));
  auto params = drop_last(std::tuple(args...));
  try {
    *last = std::apply(make_object<ObjectType>{}, params);
    return VK_SUCCESS;
  } catch (...) {
    return VK_ERROR_UNKNOWN;
  }
}

static auto CreateInstance(const VkInstanceCreateInfo *pCreateInfo,
			   const VkAllocationCallbacks *pAllocator,
			   Instance *pObject) -> VkResult {
  return Create(pCreateInfo, pAllocator, pObject);
}

static auto CreateInstance(const VkInstanceCreateInfo *pCreateInfo,
			   const VkAllocationCallbacks *pAllocator)
    -> Instance {
  Instance handle;
  auto res = CreateInstance(pCreateInfo, pAllocator, &handle);
  if (res != VK_SUCCESS) {
    throw std::runtime_error("failed to create instance");
  }
  return handle;
}

} // namespace vkx
