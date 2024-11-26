#include "adun/Parser/SelectCommand.hpp"
#include "adun/Database.hpp"
#include "adun/Parser/Command.hpp"
#include "adun/Value.hpp"
#include <fmt/format.h>

namespace adun::ast {

auto SelectCommand::execute(Database& db) -> Result {
  if (!db.m_Tables.contains(m_TableName)) {
    throw CommandException{ fmt::format("Table '{}' does not exist",
                                        m_TableName) };
  }

  ColumnNameIndexMap columnMap;
  Selector filter{ [this, &db](auto row) {
    auto evalCond{ m_Condition->evaluate(
        row, db.m_Tables.at(m_TableName).getColumnMap()) };
    if (evalCond.getType() != ValueType::Boolean) {
      throw CommandException{ fmt::format(
          "Condition should evalueate to bool, got {} instead",
          evalCond.toString()) };
    }
    return evalCond.template get<bool>();
  } };

  return db.m_Tables.at(m_TableName).selectRows(filter, m_Columns);
}

} // namespace adun::ast
