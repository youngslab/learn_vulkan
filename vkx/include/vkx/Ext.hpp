#pragma once

#include <vulkan/vulkan.hpp>
#include "vkx/Tmp.hpp"

namespace vkx {

template <typename Func> auto MakeExtensionFunction() -> Func {
  return [](auto... args) {
    auto dep = get_first(args...);
    auto func =
	(Func)vkGetInstanceProcAddr(dep, "vkCreateDebugReportCallbackEXT");
    return func(args...);
  };
}

constexpr auto MakeCreateFunctionExt() -> PFN_vkCreateDebugReportCallbackEXT {
  return [](auto... args) {
    auto dep = get_first(args...);
    auto func = (PFN_vkCreateDebugReportCallbackEXT)vkGetInstanceProcAddr(
	dep, "vkCreateDebugReportCallbackEXT");

    return func(args...);
  };
}

constexpr auto MakeDestroyFunctionExt() -> PFN_vkDestroyDebugReportCallbackEXT {
  return [](auto... args) {
    auto dep = get_first(args...);
    auto func = (PFN_vkDestroyDebugReportCallbackEXT)vkGetInstanceProcAddr(
	dep, "vkDestroyDebugReportCallbackEXT");
    return func(args...);
  };
}

} // namespace vkx
