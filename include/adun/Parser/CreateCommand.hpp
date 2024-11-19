#pragma once
#include "adun/Parser/ASTNode.hpp"
#include "adun/Parser/Command.hpp"
#include "adun/Table.hpp"

namespace adun::ast {

class CreateCommand : public Command {
public:
  CreateCommand(std::string tableName, Table::Header scheme)
      : Command{ NodeKind::CreateCommand },
        m_TableName{ std::move(tableName) },
        m_Scheme{ std::move(scheme) } {
  }

  auto execute(Database& db) -> Result override;

private:
  std::string m_TableName;
  Table::Header m_Scheme;
};

} // namespace adun::ast
