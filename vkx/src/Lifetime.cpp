

#include <vkx/Lifetime.hpp>

namespace vkx {

auto CreateAutoDeletable(uint32_t w, uint32_t h, std::string title)
    -> AutoDeletable<GLFWwindow *> {
  auto handle = CreateResource<GLFWwindow *>(w, h, title);
  auto deleter = [](GLFWwindow *w) { DeleteResource<GLFWwindow *>(w); };
  return MakeAutoDeletable<GLFWwindow *>(handle, deleter);
}

auto CreateAutoDeletable(const VkInstanceCreateInfo *pCreateInfo,
			 const VkAllocationCallbacks *pAllocator)
    -> AutoDeletable<VkInstance> {
  auto handle = CreateResource<VkInstance>(pCreateInfo, pAllocator);
  auto deleter = [pAllocator](VkInstance handle) {
    DeleteResource<VkInstance>(handle, pAllocator);
  };
  return MakeAutoDeletable<VkInstance>(handle, deleter);
}
} // namespace vkx
