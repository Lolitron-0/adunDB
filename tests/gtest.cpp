#include "adun/Column.hpp"
#include "adun/Database.hpp"
#include "adun/Exceptions.hpp"
#include "adun/Parser/Command.hpp"
#include "adun/Parser/Lexer.hpp"
#include "adun/Table.hpp"
#include "adun/Value.hpp"
#include <gtest/gtest.h>
#include <optional>

using namespace adun;      // NOLINT
using namespace adun::ast; // NOLINT
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
               NoSuchColumnException);
}

TEST(Lexer, EscapeSequences) {
  Lexer lexer;
  EXPECT_NO_THROW(lexer.lex(R"("\a")"));
  EXPECT_NO_THROW(lexer.lex(R"("\b")"));
  EXPECT_NO_THROW(lexer.lex(R"("\f")"));
  EXPECT_NO_THROW(lexer.lex(R"("\n")"));
  EXPECT_NO_THROW(lexer.lex(R"("\r")"));
  EXPECT_NO_THROW(lexer.lex(R"("\t")"));
  EXPECT_NO_THROW(lexer.lex(R"("\v")"));
  EXPECT_NO_THROW(lexer.lex(R"("\0")"));
  EXPECT_NO_THROW(lexer.lex(R"("\\")"));

  // no such sequence
  EXPECT_THROW(lexer.lex(R"("\]")"), LexerFatalError);
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

  // no such column
  EXPECT_THROW(db.execute("INSERT (non_existent = 19) INTO test;"),
               NoSuchColumnException);

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
  EXPECT_NO_THROW(db.execute(
      R"(SELECT * FROM test WHERE |name| = 3 || |"balls"| > -3;)"));

  // no such table
  EXPECT_THROW(db.execute(R"(SELECT * FROM non_existent;)"),
               CommandException);
  // invalid condition
  EXPECT_THROW(db.execute(R"(SELECT * FROM test WHERE 5;)"),
               CommandException);

  // syntax errors
  EXPECT_THROW(db.execute(R"(SELECT * test;)"), ParserException);
  EXPECT_THROW(db.execute(R"(select * from ;)"), ParserException);
  EXPECT_THROW(db.execute(R"(select * from test something_else;)"),
               ParserException);
}

TEST(Database, Update) {
  Database db;
  db.execute("create table test (id integer autoincrement, name string "
             "unique);");
  db.execute("insert (name = \"Ann\") into test;");
  db.execute("insert (name = \"Bob\") into test;");

  Result r;
  EXPECT_NO_THROW(
      r = db.execute(
          R"(update test set (name = "Alice") where id = 1;)"));
  EXPECT_EQ(r.getNumAffectedRows(), 1);
  EXPECT_NO_THROW(db.execute(
      R"(update test set (name = "_" + name +"_") where name = "Bob";)"));
  EXPECT_NO_THROW(db.execute(R"(update test set () where true;)"));

  // no such table
  EXPECT_THROW(db.execute(R"(update non_existent set () where true;)"),
               CommandException);

  // no such column
  EXPECT_THROW(
      db.execute(R"(update test set (non_existent = "Ann") where true;)"),
      NoSuchColumnException);

  // constraint break
  EXPECT_THROW(
      db.execute(R"(update test set (name = "Alice") where true;)"),
      InvalidRowException);
  EXPECT_THROW(
      db.execute(R"(update test set (id = 5, name="Ann") where id = 1;)"),
      CommandException);

  // syntax errors
  EXPECT_THROW(db.execute(R"(update test set (name = "ann") where  1;)"),
               CommandException);
  EXPECT_THROW(
      db.execute(R"(update test set (name = "ann" some, ) where true;)"),
      ParserException);
}

TEST(Database, Delete) {
  Database db;
  db.execute("create table test (id integer autoincrement, name string "
             "unique);");
  db.execute("insert (name = \"Ann\") into test;");
  db.execute("insert (name = \"Bob\") into test;");

  Result r;
  EXPECT_NO_THROW(r = db.execute(R"(delete from test where id = 1;)"));
  EXPECT_EQ(r.getNumAffectedRows(), 1);
  EXPECT_NO_THROW(db.execute(R"(delete from test  where name = "Bob";)"));

  db.execute("insert (name = \"Ann\") into test;");
  db.execute("insert (name = \"Bob\") into test;");

  // no such table
  EXPECT_THROW(db.execute(R"(delete from non_existent where true;)"),
               CommandException);

  // syntax errors
  EXPECT_THROW(db.execute(R"(delete from test where (2+2)*2;)"),
               CommandException);
}

TEST(Result, Iterate) {
  Database db;
  createTestTable(db);
  db.execute(R"(INSERT (name="Ann", age=19, data=0X003FDB) INTO test;)");
  db.execute(R"(INSERT (name="Ann", age=19, data=0X003FDA) INTO test;)");
  ByteArray expected{ 0x00, 0x3f, 0xdb };

  auto r{ db.execute(R"(SELECT * FROM test WHERE name="Ann";)") };
  for (const auto& row : r) {
    EXPECT_LE(row["id"], 2);
    EXPECT_EQ(row["age"], 19);
    EXPECT_LE(row["data"], expected);
  }

  r = db.execute(R"(SELECT * FROM test WHERE id = 1;)");
  for (auto it{ r.begin() }; it != r.end(); it++) {
    EXPECT_EQ(it->get("id"), 1);
    EXPECT_EQ(it->get("name"), "Ann");
    EXPECT_EQ(it->get("age"), 19);
    EXPECT_EQ(it->get("data"), expected);
  }
}

TEST(Value, OperatorsInt) {
  Value v1{ 5 };
  Value v2{ 10 };
  EXPECT_TRUE(v1 < v2);
  EXPECT_TRUE(v1 <= v2);
  EXPECT_FALSE(v1 == v2);
  EXPECT_FALSE(v1 > v2);
  EXPECT_FALSE(v1 >= v2);
  EXPECT_EQ(v1 - v2, -5);
  EXPECT_EQ(v1 + v2, 15);
  EXPECT_EQ(-v1, -5);
  EXPECT_EQ(v1 * v2, 50);
  EXPECT_EQ(v1 / v2, 0);
  EXPECT_EQ(v2 % v1, 0);
  EXPECT_THROW(v2 && v1, ValueException);
  EXPECT_THROW(v2 || v1, ValueException);
  EXPECT_THROW(v2 ^ v1, ValueException);
  EXPECT_THROW(!v2, ValueException);
}

TEST(Value, OperatorsString) {
  Value v1{ "Ann" };
  Value v2{ "Bob" };
  EXPECT_TRUE(v1 < v2);
  EXPECT_TRUE(v1 <= v2);
  EXPECT_FALSE(v1 == v2);
  EXPECT_FALSE(v1 > v2);
  EXPECT_FALSE(v1 >= v2);
  EXPECT_EQ(v1 + v2, "AnnBob");
  EXPECT_THROW(v1 - v2, ValueException);
  EXPECT_THROW(-v1, ValueException);
  EXPECT_THROW(v1 * v2, ValueException);
  EXPECT_THROW(v1 / v2, ValueException);
  EXPECT_THROW(v2 % v1, ValueException);
  EXPECT_THROW(v2 && v1, ValueException);
  EXPECT_THROW(v2 || v1, ValueException);
  EXPECT_THROW(v2 ^ v1, ValueException);
}

TEST(Value, OperatorsBool) {
  Value v1{ true };
  Value v2{ false };
  EXPECT_TRUE(v1 > v2);
  EXPECT_EQ(!v2, true);
  EXPECT_EQ(!v1, false);
  EXPECT_TRUE(v1 >= v2);
  EXPECT_FALSE(v1 == v2);
  EXPECT_FALSE(v1 < v2);
  EXPECT_FALSE(v1 <= v2);
  EXPECT_THROW(v1 + v2, ValueException);
  EXPECT_THROW(v1 - v2, ValueException);
  EXPECT_THROW(-v1, ValueException);
  EXPECT_THROW(v1 * v2, ValueException);
  EXPECT_THROW(v1 / v2, ValueException);
  EXPECT_THROW(v2 % v1, ValueException);
  EXPECT_FALSE(v2 && v1);
  EXPECT_TRUE(v2 || v1);
  EXPECT_TRUE(v2 ^ v1);
}
