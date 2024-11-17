#pragma once
#include "adun/Assert.hpp"
#include <concepts>
#include <cstdint>
#include <fmt/format.h>
#include <string>
#include <type_traits>
#include <variant>
#include <vector>

template <>
struct fmt::formatter<std::monostate> : fmt::formatter<std::string_view> {
  auto format(std::monostate,
              format_context& ctx) const -> format_context::iterator;
};

namespace adun {

enum class ValueType {
  Integer,
  Boolean,
  String,
  Binary,
  None,
};

namespace internal {

using DBValueUnion = std::variant<int32_t, bool, std::string,
                                  std::vector<uint8_t>, std::monostate>;

template <typename T, typename V>
struct isVariantMember;

template <typename T, typename... Ts>
struct isVariantMember<T, std::variant<Ts...>>
    : public std::disjunction<std::is_convertible<T, Ts>...> {};

template <typename T>
concept IsDBValue = isVariantMember<T, DBValueUnion>::value;

template <typename T>
struct TypeToEnumMap {
  static_assert(false, "Conversion not implemented");
};

template <std::integral T>
struct TypeToEnumMap<T> {
  static constexpr ValueType value = ValueType::Integer;
};

template <typename T>
  requires std::is_convertible_v<T, std::string> ||
           std::is_convertible_v<T, const char*>
struct TypeToEnumMap<T> {
  static constexpr ValueType value = ValueType::String;
};

template <>
struct TypeToEnumMap<bool> {
  static constexpr ValueType value = ValueType::Boolean;
};

template <>
struct TypeToEnumMap<std::vector<uint8_t>> {
  static constexpr ValueType value = ValueType::Binary;
};

} // namespace internal

class Value {
public:
  template <internal::IsDBValue T>
  /* implicit */ Value(T&& value) // NOLINT
      : m_Data{ std::forward<T>(value) },
        m_Type{ internal::TypeToEnumMap<T>::value } {
  }

  explicit Value(ValueType type)
      : m_Data{ std::monostate{} },
        m_Type{ type } {
  }

  Value()
      : m_Data{ std::monostate{} },
        m_Type{ ValueType::None } {
  }

  template <internal::IsDBValue T>
  auto get() const -> T {
    adun_assert(m_Type != ValueType::None, "Cannot get empty value");
    return std::get<T>(m_Data);
  }

  template <typename F>
  auto visit(F&& f) {
    adun_assert(m_Type != ValueType::None, "Cannot visit empty value");
    return std::visit(f, m_Data);
  }

  template <typename F>
  auto visit(F&& f) const {
    adun_assert(m_Type != ValueType::None, "Cannot visit empty value");
    return std::visit(f, m_Data);
  }

  [[nodiscard]] auto getType() const -> ValueType {
    return m_Type;
  }

  [[nodiscard]] auto toString() const -> std::string;

  static auto typeToString(ValueType type) -> std::string_view;

private:
  internal::DBValueUnion m_Data;
  ValueType m_Type;
};

} // namespace adun
