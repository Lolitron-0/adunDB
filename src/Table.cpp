#include "adun/Table.hpp"
#include "adun/Assert.hpp"
#include "adun/Exceptions.hpp"
#include "adun/Result.hpp"
#include <fmt/format.h>
#include <ranges>

namespace adun {

InvalidRowException::InvalidRowException(const std::string& msg)
    : TableException{ msg } {
}

Table::Table(std::string name, Scheme scheme)
    : m_Name{ std::move(name) },
      m_Header{ std::move(scheme) } {
  int i{ 0 };
  for (auto&& [colName, column] : m_Header) {
    adun_assert(!column.sampleValue.isEmpty(),
                "Invalid column in scheme");

    column.index = i;
    i++;
  }
};

auto Table::getName() const -> std::string {
  return m_Name;
}

auto Table::getScheme() const -> const Scheme& {
  return m_Header;
}

auto Table::selectRows(const Selector& filter,
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
      throw NoSuchColumnException(columnName);
    }
    columnMap[columnName] = m_Header.at(columnName).index;
  }
  return Result{ std::move(rows), std::move(columnMap), rows.size() };
}

void Table::traverseRows(const Selector& filter,
                         const std::function<void(Row&)>& callback) {
  for (auto&& row : m_Rows) {
    if (filter(row)) {
      callback(row);
    }
  }
}

auto Table::deleteRows(const Selector& filter) -> size_t {
  size_t affectedRows{ 0 };
  for (auto it{ m_Rows.begin() }; it != m_Rows.end();) {
    if (filter(*it)) {
      it = m_Rows.erase(it);
      affectedRows++;
    } else {
      ++it;
    }
  }
  return affectedRows;
}

void Table::addRow(
    const std::vector<std::pair<std::string, Value>>& assignments) {
  std::vector<Value> values;
  values.resize(m_Header.size());
  for (auto& [name, col] : m_Header) {
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
  for (const auto& [name, val] : assignments) {
    if (!m_Header.contains(name)) {
      throw NoSuchColumnException(name);
    }
    auto column{ m_Header.at(name) };

    if (val.getType() != column.getType()) {
      throw InvalidRowException(fmt::format(
          "Invalid value type for value {}, expected {}", val.toString(),
          Value::typeToString(column.getType())));
    }

    // auto-increment
    if (column.modifiers & Column::Modifier::AutoIncrement) {
      throw InvalidRowException(fmt::format(
          "Auto increment column {} should not be assigned", name));
    }

    values[column.index] = val;
  }

  if (std::ranges::find_if(values, [](const auto& val) {
        return val.isEmpty();
      }) != values.end()) {
    /// @todo get name of missing column
    throw InvalidRowException(fmt::format("Column missing values"));
  }

  m_Rows.emplace_back(values);
  checkConstraintsAgainst(m_Rows.back());
}

auto Table::getColumnMap() const -> const ColumnNameIndexMap& {
  if (m_ColumnMap.empty()) {
    for (auto&& [columnName, column] : m_Header) {
      m_ColumnMap[columnName] = column.index;
    }
  }
  return m_ColumnMap;
}

void Table::checkConstraintsAgainst(const Row& row) const {
  for (auto&& [columnName, column] : m_Header) {
    if (column.modifiers & Column::Modifier::Unique &&
        std::ranges::find_if(m_Rows, [index = column.index,
                                      val   = row.get(column.index),
                                      &row](const auto& otherRow) {
          // hacky, but i'm lazy to implement Key
          // comlumn modifier
          if (&row == &otherRow) {
            return false;
          }
          return otherRow.get(index) == val;
        }) != m_Rows.end()) {
      throw InvalidRowException(
          fmt::format("Value {} is not unique", columnName));
    }
  }
}

} // namespace adun
