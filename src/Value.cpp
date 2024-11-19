#include "adun/Value.hpp"
#include "adun/Assert.hpp"
#include <cul/cul.hpp>
#include <fmt/format.h>
#include <fmt/ranges.h>

template <>
struct fmt::formatter<std::monostate> : fmt::formatter<std::string_view> {
  auto format(std::monostate,
              format_context& ctx) const -> format_context::iterator {
    return formatter<std::string_view>::format("NULL", ctx);
  }
};

namespace adun {

static constexpr cul::BiMap s_TypeToString{ [](auto&& selector) {
  return selector.Case(ValueType::Integer, "Integer")
      .Case(ValueType::Boolean, "Boolean")
      .Case(ValueType::String, "String")
      .Case(ValueType::Binary, "Binary");
} };

auto Value::toString() const -> std::string {
  return visit([this](auto&& v) {
    return fmt::format("{} ({})", v, typeToString(getType()));
  });
}

auto Value::typeToString(ValueType type) -> std::string_view {
  adun_assert(s_TypeToString.FindByFirst(type).has_value(),
              "Type to string not implemented");
  return s_TypeToString.FindByFirst(type).value();
}

} // namespace adun
