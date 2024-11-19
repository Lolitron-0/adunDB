#pragma once
#include "adun/ResultIterator.hpp"
#include "adun/Types.hpp"
#include <vector>

namespace adun {

class Result {
public:
  Result() = default;
  Result(std::vector<RowRefWrapper> rows, ColumnNameIndexMap columnNames,
         size_t affectedRows);

  auto begin() -> ResultIterator;
  auto end() -> ResultIterator;

  [[nodiscard]] auto affectedRows() const -> size_t {
    return m_AffectedRows;
  }

private:
  std::vector<RowRefWrapper> m_Rows;
  ColumnNameIndexMap m_ColumnNames;
  size_t m_AffectedRows{ 0 };
};

} // namespace adun
