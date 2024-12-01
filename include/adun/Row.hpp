#pragma once
#include "adun/Value.hpp"
#include <cstddef>
#include <vector>

namespace adun {

class Row {
public:
  explicit Row(std::vector<Value> values)
      : m_Values{ std::move(values) } {
  }

  [[nodiscard]] auto get(size_t index) const -> Value {
    return m_Values[index];
    ;
  }

  [[nodiscard]] auto get(size_t index) -> Value& {
    return m_Values[index];
    ;
  }

private:
  std::vector<Value> m_Values;
};

} // namespace adun
