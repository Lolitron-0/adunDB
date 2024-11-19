#include "adun/Parser/CreateCommand.hpp"
#include "adun/Database.hpp"

namespace adun::ast {

auto CreateCommand::execute(Database& db) -> Result {
  db.m_Tables.insert(
      std::make_pair(m_TableName, Table{ m_TableName, m_Scheme }));
  return Result{};
}

} // namespace adun::ast
