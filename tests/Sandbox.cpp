#include "adun/Database.hpp"
#include "adun/Table.hpp"
#include "adun/Value.hpp"
#include <iostream>

auto main() -> int {
  adun::Database db;
  db.execute("CREATE TABLE test (id INTEGER AUTOINCREMENT, name STRING "
             "UNIQUE);");
  return 0;
}
