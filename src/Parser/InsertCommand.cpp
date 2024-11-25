#include "adun/Parser/InsertCommand.hpp"
#include "adun/Database.hpp"
#include "adun/Parser/Command.hpp"
#include <fmt/format.h>

namespace adun::ast {

auto InsertCommand::execute(Database& db) -> Result {
  if (!db.m_Tables.contains(m_TableName)) {
    throw CommandException{ fmt::format("Table '{}' does not exist",
                                        m_TableName) };
  }
  db.m_Tables.at(m_TableName).addRow(m_Values);
  return Result{ {}, {}, 1 };
}

} // namespace adun::ast
