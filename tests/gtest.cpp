#include "adun/Column.hpp"
#include "adun/Database.hpp"
#include "adun/Exceptions.hpp"
#include "adun/Parser/Command.hpp"
#include "adun/Table.hpp"
#include "adun/Value.hpp"
#include <gtest/gtest.h>
#include <optional>

using namespace adun; // NOLINT
using ColMod = Column::Modifier;

void createTestTable(Database& db) {
  db.execute(
      R"(CREATE TABLE test (id INTEGER AUTOINCREMENT, name STRING DEFAULT("Ann"), age INTEGER , data BYTE UNIQUE);)");
}

// Should cover all normal usages of insert
void insertIntoTestTable(Database& db) {
  db.execute(R"(INSERT (age=19, data=0X003F) INTO test;)");
  db.execute(R"(INSERT (name="Bob", age=20, data=0X15C) INTO test;)");
  db.execute(
      R"(INSERT (name="Luffy", age=21, data=0XFFFABC16) INTO test;)");
  db.execute(
      R"(INSERT (name="Robin", age=22, data=0X186740d8) INTO test;)");
  db.execute(
      R"(INSERT (name="Nami", age=22, data=0Xdeadbeefdeadbeefdeadbeef) INTO test;)");
}

Table::Scheme testScheme = {
  { "id", Column{ ValueType::Integer,
                  ColMod::AutoIncrement | ColMod::Unique } },
  { "name", Column{ ValueType::String, ColMod::Unique } },
  { "age", Column{ 18, ColMod::HasDefault } }
};

TEST(Table, Creation) {
  Table::Scheme scheme;
  scheme["id"]   = Column{ ValueType::Integer,
                         ColMod::AutoIncrement | ColMod::Unique };
  scheme["name"] = Column{ "default", ColMod::HasDefault };
  scheme["age"]  = Column{ ValueType::Integer };

  std::optional<Table> tbl;
  EXPECT_NO_THROW(tbl = Table("test", scheme));
  EXPECT_EQ(tbl->getName(), "test");
}

TEST(Table, Insert) {
  Table tbl("test", testScheme);
  EXPECT_NO_THROW(tbl.addRow({ { "name", "Ann" }, { "age", 30 } }));
  EXPECT_NO_THROW(tbl.addRow({ { "name", "Bob" } }));
  EXPECT_NO_THROW(tbl.addRow({ { "name", "Cat" } }));

  // Missing values
  EXPECT_ANY_THROW(tbl.addRow({}));

  // Wrong column
  EXPECT_ANY_THROW(tbl.addRow({ { "non-existent", 1 } }));

  // Wrong types
  EXPECT_ANY_THROW(tbl.addRow({ { "name", "Rob2" }, { "age", "20" } }));

  // Non unique
  EXPECT_ANY_THROW(tbl.addRow({ { "name", "Ann" }, { "age", 20 } }));

  // Auto increment assign
  EXPECT_ANY_THROW(
      tbl.addRow({ { "id", 185 }, { "age", 20 }, { "name", "Rin" } }));
}

TEST(Table, Select) {
  Table tbl("test", testScheme);
  tbl.addRow({ { "name", "Ann" }, { "age", 30 } });
  tbl.addRow({ { "name", "Bob" } });
  tbl.addRow({ { "name", "Cat" } });
  EXPECT_NO_THROW(tbl.selectRows(
      [](const auto& /*row*/) {
        return true;
      },
      { "name", "age" }));
  EXPECT_NO_THROW(tbl.selectRows(
      [](const auto& row) {
        return row.get(0) == "1";
      },
      { "name", "age" }));
  EXPECT_THROW(tbl.selectRows(
                   [](const auto& /*row*/) {
                     return true;
                   },
                   { "name", "age", "non-existent" }),
               InvalidRowException);
}

TEST(Column, Creation) {
  EXPECT_NO_THROW(Column col("a", ColMod::HasDefault));
  EXPECT_NO_THROW(Column col("a", ColMod::Unique));
  EXPECT_NO_THROW(Column col(1, ColMod::AutoIncrement));
  EXPECT_ANY_THROW(Column col("a", ColMod::AutoIncrement));
  EXPECT_ANY_THROW(Column col(
      ValueType::Integer, ColMod::AutoIncrement | ColMod::HasDefault));
  EXPECT_ANY_THROW(Column col("a", ColMod::Unique | ColMod::HasDefault));
}

TEST(Database, Create) {
  Database db;
  EXPECT_NO_THROW(db.execute("CREATE TABLE test1 (id INTEGER);"));
  EXPECT_NO_THROW(
      db.execute("CREATE TABLE test2 (id INTEGER, name STRING);"));
  EXPECT_NO_THROW(db.execute(
      "CREATE TABLE test3 (id INTEGER AUTOINCREMENT, name STRING);"));
  EXPECT_NO_THROW(
      db.execute("CREATE TABLE test4 (id INTEGER AUTOINCREMENT, name "
                 "STRING UNIQUE, data BYTE DEFAULT(0X003F));"));

  // exists
  EXPECT_ANY_THROW(
      db.execute("CREATE TABLE test1 (id INTEGER AUTOINCREMENT);"));

  // syntax errors
  EXPECT_ANY_THROW(
      db.execute("CREATE test0 (id INTEGER AUTOINCREMENT);"));
  EXPECT_ANY_THROW(
      db.execute("CREATE TABLE (id INTEGER AUTOINCREMENT);"));
  EXPECT_ANY_THROW(db.execute(
      "CREATE TABLE test0 id INTEGER AUTOINCREMENT, name STRING;"));
  EXPECT_ANY_THROW(db.execute("CREATE TABLE test0 ();"));
  EXPECT_ANY_THROW(db.execute("CREATE TABLE test0 (INTEGER);"));
  EXPECT_ANY_THROW(
      db.execute("CREATE TABLE test0 (id INTEGER, id STRING);"));
  EXPECT_ANY_THROW(db.execute("CREATE TABLE test0 (id, name STRING);"));
  EXPECT_ANY_THROW(
      db.execute("CREATE TABLE test0 (id INTEGER DEFAULT 3);"));
  EXPECT_ANY_THROW(
      db.execute("CREATE TABLE test0 (id INTEGER DEFAULT(NULL));"));
  EXPECT_ANY_THROW(
      db.execute("CREATE TABLE test0 (id INTEGER DEFAULT(\"hehe\"));"));
  EXPECT_ANY_THROW(
      db.execute("CREATE TABLE test0 (id INTEGER DEFAULT(1;"));
  EXPECT_ANY_THROW(
      db.execute("CREATE TABLE test0 (id INTEGER NON-EXISTENT);"));
  EXPECT_ANY_THROW(
      db.execute("CREATE TABLE test0 (id INTEGER) something_else"));
}

TEST(Database, Insert) {
  Database db;
  createTestTable(db);

  EXPECT_NO_THROW(insertIntoTestTable(db));

  // no such table
  EXPECT_THROW(db.execute("INSERT (age = 19) INTO non_existent;"),
               CommandException);

  // syntax errors
  EXPECT_THROW(db.execute("INSERT age = 19 INTO test;"), ParserException);
  EXPECT_THROW(db.execute("INSERT ( = 19) INTO test;"), ParserException);
  EXPECT_THROW(db.execute("INSERT (age 19) INTO test;"), ParserException);
  EXPECT_THROW(db.execute("INSERT (age = ) INTO test;"), ParserException);
  EXPECT_THROW(db.execute("INSERT (age = 1)"), ParserException);
  EXPECT_THROW(db.execute("INSERT (age = 1) INTO ;"), ParserException);
  EXPECT_THROW(db.execute("INSERT (age = 1) INTO test hello;"),
               ParserException);
}

TEST(Database, Select) {
  Database db;
  createTestTable(db);
  insertIntoTestTable(db);

  EXPECT_NO_THROW(db.execute(R"(SELECT * FROM test;)"));
  EXPECT_NO_THROW(db.execute(R"(SELECT * FROM test WHERE true;)"));
  EXPECT_NO_THROW(
      db.execute(R"(SELECT * FROM test WHERE age + id >= 5;)"));
  EXPECT_NO_THROW(
      db.execute(R"(SELECT * FROM test WHERE data = 0X003F;)"));
  EXPECT_NO_THROW(db.execute(
      R"(SELECT * FROM test WHERE |name| = 3 || |"balls"| < 3;)"));

  // no such table
  EXPECT_THROW(db.execute(R"(SELECT * FROM non_existent;)"),
               CommandException);
  // invalid condition
  EXPECT_THROW(db.execute(R"(SELECT * FROM test WHERE 5;)"),
               CommandException);

  // syntax errors
  EXPECT_THROW(db.execute(R"(SELECT * test;)"), ParserException);
  EXPECT_THROW(db.execute(R"(select * from ;)"), ParserException);
  EXPECT_THROW(db.execute(R"(select * from test something_else;)"), ParserException);
}
