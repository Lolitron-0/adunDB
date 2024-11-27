#include "adun/Parser/CreateCommand.hpp"
#include "adun/Database.hpp"
#include <fmt/format.h>

namespace adun::ast {

auto CreateCommand::execute(Database& db) -> Result {
  if (db.m_Tables.contains(m_TableName)) {
    throw CommandException{ fmt::format("Table '{}' already exists",
                                        m_TableName) };
  }
  db.m_Tables.insert(
      std::make_pair(m_TableName, Table{ m_TableName, m_Scheme }));
  return Result{};
}

} // namespace adun::ast
