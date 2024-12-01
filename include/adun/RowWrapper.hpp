#pragma once
#include "adun/Row.hpp"
#include "adun/Types.hpp"

namespace adun {

class ResultIterator;

class RowWrapper {

public:
  RowWrapper() = default;
  RowWrapper(Row* row, ColumnNameIndexMap columnNames)
      : m_RowPtr{ row },
        m_Columns{ std::move(columnNames) } {
  }

  auto operator[](const std::string& columnName) const -> Value {
    return m_RowPtr->get(m_Columns.at(columnName));
  }

  auto get(const std::string& columnName) const -> Value {
    return this->operator[](columnName);
  }

  friend class ResultIterator;

private:
  Row* m_RowPtr{ nullptr };
  ColumnNameIndexMap m_Columns;
};

} // namespace adun
