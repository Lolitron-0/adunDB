#pragma once
#include "adun/Row.hpp"
#include "adun/Value.hpp"
#include <fmt/format.h>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <unordered_map>
#include <utility>
#include <vector>

namespace adun {

class TableException : public std::runtime_error {
protected:
  explicit TableException(const std::string& msg)
      : std::runtime_error{ msg } {
  }
};

class InvalidRowException : public TableException {
public:
  explicit InvalidRowException(const std::string& msg)
      : TableException{ msg } {
  }
};

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

  Column(Value value, ModifierFlags flags)
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
      sampleValue = Value{ ValueType::Integer };
    }
  }

  explicit Column(ValueType type, ModifierFlags flags = {})
      : Column{ Value{ type }, flags } {
    if (flags & Modifier::HasDefault) {
      throw ColumnFormatException("No default value provided");
    }
  }

  [[nodiscard]] auto getType() const -> ValueType {
    return sampleValue.getType();
  }

  Value sampleValue; ///< serves multiple purpose as default value, type
                     ///< and autoincrement counter
  ModifierFlags modifiers{ Modifier::None };
  size_t index{ 0 };
};

class Table {
public:
  using Header = std::unordered_map<std::string, Column>;

  Table(std::string name, Header scheme)
      : m_Name{ std::move(name) },
        m_Header{ std::move(scheme) } {};

  [[nodiscard]] auto getName() const -> std::string {
    return m_Name;
  }

  void addRow(std::vector<std::pair<std::string, Value>> assignments) {
    std::vector<Value> values;
    values.resize(m_Header.size());
    for (auto&& [name, col] : m_Header) {
      if (col.modifiers & Column::Modifier::HasDefault) {
        values[col.index] = col.sampleValue;
      }
    }
    for (auto&& [name, val] : assignments) {
      if (val.getType() != m_Header.at(name).getType()) {
        throw InvalidRowException(fmt::format(
            "Invalid value type for value {}, expected {}",
            val.toString(),
            Value::typeToString(m_Header.at(name).getType())));
      }
      values[m_Header.at(name).index] = std::move(val);
    }
  }

private:
  std::string m_Name;
  Header m_Header;
  std::vector<Row> m_Rows;
};

} // namespace adun
