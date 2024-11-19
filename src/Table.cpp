#include "adun/Table.hpp"
#include "adun/Assert.hpp"
#include "adun/Result.hpp"

namespace adun {

InvalidRowException::InvalidRowException(const std::string& msg)
    : TableException{ msg } {
}

Table::Table(std::string name, Header scheme)
    : m_Name{ std::move(name) },
      m_Header{ std::move(scheme) } {
  int i{ 0 };
  for (auto&& [_, column] : m_Header) {
    adun_assert(!column.sampleValue.isEmpty(),
                "Invalid column in scheme");
    column.index = i;
    i++;
  }
};

auto Table::getName() const -> std::string {
  return m_Name;
}

auto Table::selectRows(const std::function<bool(const Row&)>& filter,
                       const std::vector<std::string>& columns)
    -> Result {
  std::vector<RowRefWrapper> rows;
  for (auto&& row : m_Rows) {
    if (filter(row)) {
      rows.emplace_back(row);
    }
  }
  ColumnNameIndexMap columnMap;
  for (auto&& columnName : columns) {
    if (!m_Header.contains(columnName)) {
      throw InvalidRowException(
          fmt::format("No such column: {}", columnName));
    }
    columnMap[columnName] = m_Header.at(columnName).index;
  }
  return Result{ std::move(rows), std::move(columnMap) };
}

void Table::addRow(
    std::vector<std::pair<std::string, Value>> assignments) {
  std::vector<Value> values;
  values.resize(m_Header.size());
  for (auto&& [name, col] : m_Header) {
    if (col.modifiers & Column::Modifier::HasDefault) {
      values[col.index] = col.sampleValue;
    }

    if (col.modifiers & Column::Modifier::AutoIncrement) {
      adun_assert(col.getType() == ValueType::Integer,
                  "Autoincrement on non integer column");
      col.sampleValue.set(col.sampleValue.get<int32_t>() + 1);
      values[col.index] = col.sampleValue.get<int32_t>();
    }
  }
  for (auto&& [name, val] : assignments) {
    if (!m_Header.contains(name)) {
      throw InvalidRowException(fmt::format("No such column: {}", name));
    }
    auto column{ m_Header.at(name) };

    if (val.getType() != column.getType()) {
      throw InvalidRowException(fmt::format(
          "Invalid value type for value {}, expected {}", val.toString(),
          Value::typeToString(column.getType())));
    }

    // uniqueness
    if (column.modifiers & Column::Modifier::Unique &&
        /// @todo faster unique check
        std::ranges::find_if(m_Rows, [&column, &val](const auto& row) {
          return row.get(column.index) == val;
        }) != m_Rows.end()) {
      throw InvalidRowException(
          fmt::format("Value {} is not unique", name));
    }

    // auto-increment
    if (column.modifiers & Column::Modifier::AutoIncrement) {
      throw InvalidRowException(fmt::format(
          "Auto increment column {} should not be assigned", name));
    }

    values[column.index] = std::move(val);
  }

  if (std::ranges::find_if(values, [](const auto& val) {
        return val.isEmpty();
      }) != values.end()) {
    /// @todo get name of missing column
    throw InvalidRowException(fmt::format("Column missing values"));
  }

  m_Rows.emplace_back(values);
}

} // namespace adun
