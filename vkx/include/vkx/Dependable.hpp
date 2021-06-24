
#pragma once

#include <any>
#include <vulkan/vulkan.hpp>
#include "vkx/Tmp.hpp"

namespace vkx {

class Dependable {
private:
  std::shared_ptr<std::vector<std::any>> _storage;

public:
  Dependable() : _storage(std::make_shared<std::vector<std::any>>()) {}
  Dependable(Dependable const &rhs) : _storage(rhs._storage) {}

  template <typename T> auto Depend(T object) -> void {
    _storage->push_back(object);
  }
};
} // namespace vkx
