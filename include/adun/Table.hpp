#pragma once
#include "adun/Column.hpp"
#include "adun/Exceptions.hpp"
#include "adun/Result.hpp"
#include "adun/Row.hpp"
#include "adun/Types.hpp"
#include "adun/Value.hpp"
#include <fmt/format.h>
#include <functional>
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
  using Header = std::unordered_map<std::string, Column>;

  Table(std::string name, Header scheme);

  [[nodiscard]] auto getName() const -> std::string;

  auto selectRows(const std::function<bool(const Row&)>& filter,
                  const std::vector<std::string>& columns) -> Result;

  void addRow(const std::vector<std::pair<std::string, Value>>& assignments);

private:
  std::string m_Name;
  Header m_Header;
  std::vector<Row> m_Rows;
};

} // namespace adun
