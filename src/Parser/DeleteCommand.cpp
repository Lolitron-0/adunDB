#include "adun/Parser/DeleteCommand.hpp"
#include "adun/Database.hpp"
#include "adun/Parser/Command.hpp"
#include <fmt/format.h>

namespace adun::ast {

auto DeleteCommand::execute(Database& db) -> Result {
  if (!db.m_Tables.contains(m_TableName)) {
    throw CommandException{ fmt::format("Table '{}' does not exist",
                                        m_TableName) };
  }

  Table& table{ db.m_Tables.at(m_TableName) };
  Selector filter{ [this, &table](auto row) {
    auto evalCond{ m_Condition->evaluate(row, table.getColumnMap()) };
    if (evalCond.getType() != ValueType::Boolean) {
      throw CommandException{ fmt::format(
          "Condition should evalueate to bool, got {} instead",
          evalCond.toString()) };
    }
    return evalCond.template get<bool>();
  } };

  auto affectedRows{ table.deleteRows(filter) };
  return Result{ {}, {}, affectedRows };
}

} // namespace adun::ast
