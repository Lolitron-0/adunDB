#include "adun/Database.hpp"

#include "adun/Exceptions.hpp"
#include <iostream>

auto main() -> int {
  adun::Database db;
  try {
    db.execute(
        R"(CREATE TABLE test (id INTEGER AUTOINCREMENT, name STRING DEFAULT("Ann"), age INTEGER UNIQUE);)");
    db.execute(R"(INSERT (age = 19,name="baby") INTO test; )");
    db.execute(R"(INSERT (age = 20, name="John") INTO test; )");
    db.execute(R"(INSERT (age = 21, name="Jane") INTO test; )");
    db.execute(R"(INSERT (age = 22, name="Mary") INTO test; )");
    db.execute(R"(INSERT (age = 23) INTO test; )");
    auto r{ db.execute(
        R"(SELECT id,name,age FROM test WHERE 2+2*2=6; )") };
    for (const auto& row : r) {
      std::cout << row["id"] << ' ' << row["name"] << ' ' << row["age"]
                << '\n';
    }
  } catch (const adun::DatabaseException& e) {
    std::cerr << e.what() << '\n';
  }
  return 0;
}
