#include "adun/ResultIterator.hpp"
#include "adun/RowWrapper.hpp"
#include <utility>

namespace adun {

ResultIterator::ResultIterator(UnderlyingIterator iter,
                               UnderlyingIterator end,
                               ColumnNameIndexMap columnNames)
    : m_Iter{ iter },
      m_EndIter{ end } {
  if (m_Iter != m_EndIter) {
    m_Row = { &m_Iter->get(), std::move(columnNames) };
  }
}
auto ResultIterator::operator->() -> pointer {
  return &m_Row;
}
auto ResultIterator::operator*() -> reference {
  return m_Row;
}
auto ResultIterator::operator++(int) -> ResultIterator {
  auto copy{ *this };
  operator++();
  return copy;
}
auto ResultIterator::operator++() -> ResultIterator& {
  ++m_Iter;
  if (m_Iter != m_EndIter) {
    m_Row = { &m_Iter->get(), std::move(m_Row.m_Columns) };
  }
  return *this;
}

auto ResultIterator::operator!=(const ResultIterator& other) const
    -> bool {
  return m_Iter != other.m_Iter;
}

} // namespace adun
