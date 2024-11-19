#pragma once
#include "adun/Exceptions.hpp"
#include "adun/Value.hpp"
#include <cstdint>

namespace adun {

class ColumnFormatException : public TableException {
public:
  explicit ColumnFormatException(const std::string& msg)
      : TableException{ msg } {
  }
};

#define BIT(x) (1U << static_cast<uint8_t>(x)) // NOLINT

class Column {
public:
  enum Modifier : uint8_t {
    None          = 0,
    AutoIncrement = BIT(0),
    Key           = BIT(1),
    Unique        = BIT(2),
    HasDefault    = BIT(3),
  };

  using ModifierFlags = std::underlying_type_t<Modifier>;

  Column();
  Column(Value value, ModifierFlags flags);

  explicit Column(ValueType type, ModifierFlags flags = {});

  [[nodiscard]] auto getType() const -> ValueType;

  Value sampleValue; ///< serves multiple purpose as default value, type
                     ///< and autoincrement counter
  ModifierFlags modifiers{ Modifier::None };
  size_t index{ 0 };
};

} // namespace adun
