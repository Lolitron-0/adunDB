#include "adun/Result.hpp"
#include "adun/ResultIterator.hpp"

namespace adun {

Result::Result(std::vector<RowRefWrapper> rows,
               ColumnNameIndexMap columnNames, size_t affectedRows)
    : m_Rows{ std::move(rows) },
      m_ColumnNames{ std::move(columnNames) },
      m_AffectedRows{ affectedRows } {
}

auto Result::begin() -> ResultIterator {
  return ResultIterator{ m_Rows.begin(), m_Rows.end(), m_ColumnNames };
}
auto Result::end() -> ResultIterator {
  return ResultIterator{ m_Rows.end(), m_Rows.end(), m_ColumnNames };
}

} // namespace adun
