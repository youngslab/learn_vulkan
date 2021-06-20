
#pragma once
#include <memory>
#include <functional>
#include <vkx/Lifetime.hpp>

namespace vkx {

// manage dependency
template <typename Resource> class Object : public AutoDeletable<Resource> {
public:
  template <typename... Args>
  Object(Args &&...args)
      : AutoDeletable<Resource>(CreateAutoDeletable(args...)) {}
};

} // namespace vkx
