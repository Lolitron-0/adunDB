#pragma once
#include "adun/RowWrapper.hpp"
#include "adun/Types.hpp"
#include <iterator>
#include <vector>

namespace adun {

class ResultIterator {
public:
  using iterator_category  = std::random_access_iterator_tag;
  using UnderlyingIterator = std::vector<RowRefWrapper>::const_iterator;
  using value_type         = const RowRefWrapper;
  using difference_type    = std::ptrdiff_t;
  using pointer            = const RowWrapper*;
  using reference          = const RowWrapper&;

  explicit ResultIterator(UnderlyingIterator iter, UnderlyingIterator end,
                          ColumnNameIndexMap columnNames);

  auto operator->() -> pointer;

  auto operator*() -> reference;

  auto operator++(int) -> ResultIterator;

  auto operator++() -> ResultIterator&;

  auto operator!=(const ResultIterator& other) const -> bool;

private:
  UnderlyingIterator m_Iter;
  UnderlyingIterator m_EndIter;
  RowWrapper m_Row;
};

} // namespace adun
