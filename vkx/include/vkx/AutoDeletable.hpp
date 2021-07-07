#pragma once

#include <memory>
#include <functional>

namespace vkx {
template <typename T> class AutoDeletable {
private:
  std::shared_ptr<T> _storage;

public:
  AutoDeletable() : _storage(nullptr) {}
  AutoDeletable(AutoDeletable const &rhs) : _storage(rhs._storage) {}
  AutoDeletable(T handle, std::function<void(T)> deleter)
      : _storage(std::shared_ptr<T>(new T(handle), [deleter](T *ptr) {
	  deleter(*ptr);
	  delete ptr;
	})) {}

  operator T() & { return *_storage; }
  operator T() && = delete;
  operator T() const & { return *_storage; }
  operator T() const && = delete;

  T *GetPointer() { return _storage.get(); }
  T GetValue() { return *this; }
};

template <typename T>
auto MakeAutoDeletable(T handle, std::function<void(T)> deleter)
    -> AutoDeletable<T> {
  return AutoDeletable(handle, deleter);
}

} // namespace vkx
