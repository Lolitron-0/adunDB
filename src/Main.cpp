#include "adun/Table.hpp"
#include "adun/Value.hpp"

auto main() -> int {
  adun::Table::Header scheme;
  scheme.insert(
      { "zhopa",
        adun::Column{ "pise", adun::Column::Modifier::HasDefault } });
  adun::Table tbl{ "test", scheme };
  tbl.addRow({ { "zhopa", "pisa" } });
}
