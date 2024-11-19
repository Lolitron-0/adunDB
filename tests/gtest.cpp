#include "adun/Column.hpp"
#include "adun/Table.hpp"
#include "adun/Value.hpp"
#include <gtest/gtest.h>
#include <optional>

using namespace adun; // NOLINT
using ColMod = Column::Modifier;

Table::Header testScheme = {
  { "id", Column{ ValueType::Integer,
                  ColMod::AutoIncrement | ColMod::Unique } },
  { "name", Column{ ValueType::String, ColMod::Unique } },
  { "age", Column{ 18, ColMod::HasDefault } }
};

TEST(Table, Creation) {
  Table::Header scheme;
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

TEST(Column, Creation) {
  EXPECT_NO_THROW(Column col("a", ColMod::HasDefault));
  EXPECT_NO_THROW(Column col("a", ColMod::Unique));
  EXPECT_NO_THROW(Column col(1, ColMod::AutoIncrement));
  EXPECT_ANY_THROW(Column col("a", ColMod::AutoIncrement));
  EXPECT_ANY_THROW(Column col(
      ValueType::Integer, ColMod::AutoIncrement | ColMod::HasDefault));
  EXPECT_ANY_THROW(Column col("a", ColMod::Unique | ColMod::HasDefault));
}
