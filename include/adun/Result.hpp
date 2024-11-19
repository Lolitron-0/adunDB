#pragma once
#include "adun/ResultIterator.hpp"
#include "adun/Types.hpp"
#include <vector>

namespace adun {

class Result {
public:
  Result(std::vector<RowRefWrapper> rows, ColumnNameIndexMap columnNames);

  auto begin() -> ResultIterator;
  auto end() -> ResultIterator;

private:
  std::vector<RowRefWrapper> m_Rows;
  ColumnNameIndexMap m_ColumnNames;
};

} // namespace adun
