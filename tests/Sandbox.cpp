#include "adun/Table.hpp"
#include "adun/Value.hpp"
#include <iostream>

auto main() -> int {
  adun::Table::Header scheme;
  scheme.insert(
      { "zhopa",
        adun::Column{ "pise", adun::Column::Modifier::HasDefault } });
  scheme.insert(
      { "id",
        adun::Column{ adun::ValueType::Integer,
                      adun::Column::Modifier::Unique |
                          adun::Column::Modifier::AutoIncrement } });
  adun::Table tbl{ "test", scheme };
  tbl.addRow({
      { "zhopa", "pisa" },
  });
  tbl.addRow({});
  auto res{ tbl.selectRows(
      [](auto&&) {
        return true;
      },
      { "id", "zhopa" }) };

  for (auto&& row : res) {
    std::cout << row["id"].get<int32_t>() << std::endl;
  }
  return 0;
}
