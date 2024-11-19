#include "adun/Column.hpp"

namespace adun {

Column::Column(Value value, ModifierFlags flags)
    : sampleValue{ std::move(value) },
      modifiers{ flags } {
  if (flags & Modifier::AutoIncrement && flags & Modifier::HasDefault) {
    throw ColumnFormatException(
        "Auto increment column cannot have default value");
  }

  if (flags & Modifier::Unique && flags & Modifier::HasDefault) {
    throw ColumnFormatException("Unique and default used together");
  }

  if (flags & Modifier::AutoIncrement) {
    if (value.getType() != ValueType::Integer) {
      throw ColumnFormatException(
          "Auto increment column must be of type integer");
    }
    if (sampleValue.isEmpty() || sampleValue.isNull()) {
      sampleValue = 0;
    }
  }
}

Column::Column()
    : Column{ Value{}, Modifier::None } {
}

Column::Column(ValueType type, ModifierFlags flags)
    : Column{ Value{ type }, flags } {
  if (flags & Modifier::HasDefault) {
    throw ColumnFormatException("No default value provided");
  }
}

auto Column::getType() const -> ValueType {
  return sampleValue.getType();
}

} // namespace adun
