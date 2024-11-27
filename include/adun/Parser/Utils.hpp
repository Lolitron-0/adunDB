#pragma once
#include <algorithm>
#include <array>
#include <memory>
#include <string>
#include <string_view>

namespace adun {

using SourceIt = std::string::const_iterator;

template <typename T>
using Ref = std::shared_ptr<T>;

template <typename T>
using Unique = std::unique_ptr<T>;

template <typename T, typename... Args>
auto makeRef(Args&&... args) -> Ref<T> {
  return std::make_shared<T>(std::forward<Args>(args)...);
}

template <typename T, typename... Args>
auto makeUnique(Args&&... args) -> Unique<T> {
  return std::make_unique<T>(std::forward<Args>(args)...);
}

void skipSpacesSince(SourceIt& pos);

auto consumeIdent(SourceIt& pos) -> std::string_view;

template <typename T, size_t N>
constexpr auto sortConstexpr(std::array<T, N> arr,
                             bool (*less)(const std::decay_t<T>&,
                                          const std::decay_t<T>&))
    -> std::array<T, N> {
  for (std::size_t i{ 0 }; i < N - 1; ++i) {
    for (std::size_t j{ i + 1 }; j < N; ++j) {
      if (less(arr[j], arr[i])) {
        std::swap(arr[i], arr[j]);
      }
    }
  }
  return arr;
}

} // namespace adun
