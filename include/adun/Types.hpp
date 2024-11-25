#pragma once
#include <functional>
#include <string>

namespace adun {

class Row;

using RowRefWrapper      = std::reference_wrapper<Row>;
using ColumnNameIndexMap = std::unordered_map<std::string, size_t>;
using Selector           = std::function<bool(const Row&)>;

} // namespace adun
