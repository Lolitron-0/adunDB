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

using ByteArray = std::vector<uint8_t>;

namespace internal {

using DBValueUnion =
    std::variant<int32_t, bool, std::string, ByteArray, std::monostate>;

template <typename T, typename V>
struct isVariantMember;

template <typename T, typename... Ts>
struct isVariantMember<T, std::variant<Ts...>>
    : public std::disjunction<std::is_convertible<T, Ts>...> {};

template <typename T>
concept IsDBValue = isVariantMember<T, DBValueUnion>::value;

template <typename T>
struct TypeToEnumMap;

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

struct HoldsMonostateVisitor {
  auto operator()(const std::monostate& /*value*/) -> bool {
    return true;
  }

  template <IsDBValue T>
  auto operator()(const T& /*value*/) -> bool {
    return false;
  }
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

  template <internal::IsDBValue T>
  void set(T&& value) {
    m_Data = std::forward<T>(value);
    m_Type = internal::TypeToEnumMap<T>::value;
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

  [[nodiscard]] auto isEmpty() const -> bool {
    return m_Type == ValueType::None;
  }

  [[nodiscard]] auto isNull() const -> bool {
    adun_assert(m_Type != ValueType::None, "Empty value usage");
    return std::visit(internal::HoldsMonostateVisitor{}, m_Data);
  }

  [[nodiscard]] auto toString() const -> std::string;

  static auto typeToString(ValueType type) -> std::string_view;

  auto operator==(const Value& other) const -> bool {
    return m_Type == other.m_Type && m_Data == other.m_Data;
  }

  auto operator<=>(const Value& other) const {
    adun_assert(m_Type == other.m_Type,
                "Cannot compare values of different types");
    return m_Data <=> other.m_Data;
  }

private:
  internal::DBValueUnion m_Data;
  ValueType m_Type;
};

} // namespace adun
