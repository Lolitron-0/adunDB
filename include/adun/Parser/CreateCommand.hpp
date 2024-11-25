#pragma once
#include "adun/Parser/ASTNode.hpp"
#include "adun/Parser/Command.hpp"
#include "adun/Table.hpp"

namespace adun::ast {

class CreateCommand final : public Command {
public:
  CreateCommand(std::string tableName, Table::Scheme scheme)
      : Command{ NodeKind::CreateCommand },
        m_TableName{ std::move(tableName) },
        m_Scheme{ std::move(scheme) } {
  }

  auto execute(Database& db) -> Result override;

private:
  std::string m_TableName;
  Table::Scheme m_Scheme;
};

} // namespace adun::ast
