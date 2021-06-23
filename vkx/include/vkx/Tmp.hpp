#pragma once

#include <tuple>

namespace vkx {
// select
template <typename Tuple, std::size_t... Ints>
auto select(Tuple &&tuple, std::index_sequence<Ints...>) {
  return std::tuple{std::get<Ints>(std::forward<Tuple>(tuple))...};
}

// last_t
template <typename... Args> struct last;
template <typename T> struct last<T> { using type = T; };
template <typename T, typename... Args>
struct last<T, Args...> : last<Args...> {};
template <typename... Args> using last_t = typename last<Args...>::type;

// get_last
template <typename... Args> auto get_last(Args... args) -> last_t<Args...> {
  return std::get<sizeof...(Args) - 1>(std::tuple{args...});
}

template <typename... Args> auto get_first(Args... args) {
  return std::get<0>(std::tuple{args...});
}

// drop_last
template <template <typename...> class Tuple, typename... Args>
auto drop_last(Tuple<Args...> const &tuple) {
  return select(tuple, std::make_index_sequence<sizeof...(Args) - 1>{});
}

template <typename... Args> auto drop_last(Args... args) {
  return drop_last(std::tuple(args...));
}

template <typename T> struct make_object {
  template <typename... Args> auto operator()(Args... args) -> T {
    return T(args...);
  }
};
} // namespace vkx
