#pragma once
#include "adun/Column.hpp"
#include "adun/Exceptions.hpp"
#include "adun/Result.hpp"
#include "adun/Row.hpp"
#include "adun/Types.hpp"
#include "adun/Value.hpp"
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

namespace adun {

class InvalidRowException : public TableException {
public:
  explicit InvalidRowException(const std::string& msg);
};

class Table {
public:
  using Scheme = std::unordered_map<std::string, Column>;

  Table(std::string name, Scheme scheme);

  [[nodiscard]] auto getName() const -> std::string;

  auto getColumnMap() const -> const ColumnNameIndexMap&;

  auto selectRows(const Selector& filter,
                  const std::vector<std::string>& columns) -> Result;

  void addRow(
      const std::vector<std::pair<std::string, Value>>& assignments);

private:
  std::string m_Name;
  Scheme m_Header;
  std::vector<Row> m_Rows;
  mutable ColumnNameIndexMap m_ColumnMap;
};

} // namespace adun
