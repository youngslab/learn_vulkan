
#pragma once

#include <vkx/Object.hpp>

namespace vkx {

auto CreateInstance(const VkInstanceCreateInfo *pCreateInfo,
		    const VkAllocationCallbacks *pAllocator, Instance *pObject)
    -> VkResult;

}
