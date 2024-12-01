#include "adun/Database.hpp"

#include "adun/Exceptions.hpp"
#include <iostream>

auto main() -> int {
  adun::Database db;
  try {
    // TODO: dublicate columns
    db.execute(
        R"(CREATE TABLE test (id INTEGER AUTOINCREMENT, name STRING DEFAULT("Ann"), age INTEGER UNIQUE, data BYTE DEFAULT(0x003F));)");
    db.execute(R"(INSERT (age = 19,name="baby") INTO test; )");
    db.execute(R"(INSERT (age = 20, name="John") INTO test; )");
    db.execute(R"(INSERT (age = 21, name="Jane") INTO test; )");
    db.execute(R"(INSERT (age = 22, name="Mary") INTO test; )");
    db.execute(R"(INSERT (age = 23) INTO test; )");
    auto r{ db.execute(R"(SELECT * FROM test; )") };
    for (const auto& row : r) {
      std::cout << row["id"] << ' ' << row["name"] << ' ' << row["age"]
                << ' ' << row["data"] << '\n';
    }
    r = db.execute("update test set (age = age+1) where name = \"Jane\"");
    r = db.execute(R"(SELECT * FROM test; )");
    for (const auto& row : r) {
      std::cout << row["id"] << ' ' << row["name"] << ' ' << row["age"]
                << ' ' << row["data"] << '\n';
    }
  } catch (const adun::DatabaseException& e) {
    std::cerr << e.what() << '\n';
  }
  return 0;
}
