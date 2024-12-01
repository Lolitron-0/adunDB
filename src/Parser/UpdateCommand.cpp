#include "adun/Parser/UpdateCommand.hpp"
#include "adun/Database.hpp"
#include "adun/Parser/Command.hpp"
#include "adun/Parser/VariableExpr.hpp"
#include <fmt/format.h>

namespace adun::ast {

auto UpdateCommand::execute(Database& db) -> Result {
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

  size_t affectedRows{ 0 };
  const auto& colMap{ table.getColumnMap() };
  table.traverseRows(filter,
                     [this, &affectedRows, &table, &colMap](auto& row) {
                       for (auto& [columnName, expr] : m_Values) {
                         if (!colMap.contains(columnName)) {
                           throw NoSuchColumnException(columnName);
                         }

                         row.get(colMap.at(columnName)) =
                             expr->evaluate(row, table.getColumnMap());
                       }
                       affectedRows++;
                     });
  return Result{ {}, {}, affectedRows };
}

} // namespace adun::ast
