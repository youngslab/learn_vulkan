#pragma once

#include <vulkan/vulkan.hpp>
#include "vkx/Tmp.hpp"

namespace vkx {

template <typename Func, char const *Name>
constexpr auto LoadExtFunction() -> Func {
  return [](auto... args) {
    auto dep = get_first(args...);
    auto func = (Func)vkGetInstanceProcAddr(dep, Name);
    return func(args...);
  };
}

} // namespace vkx
