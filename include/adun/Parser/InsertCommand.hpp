#pragma once
#include "adun/Parser/Command.hpp"

namespace adun::ast {

class InsertCommand final : public Command {
public:
  InsertCommand(std::string tableName,
                std::vector<std::pair<std::string, Value>> values)
      : Command{ NodeKind::InsertCommand },
        m_TableName{ std::move(tableName) },
        m_Values{ std::move(values) } {
  }

  auto execute(Database& db) -> Result override;

private:
  std::string m_TableName;
  std::vector<std::pair<std::string, Value>> m_Values;
};

} // namespace adun::ast
