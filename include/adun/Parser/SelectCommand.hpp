#pragma once
#include "adun/Parser/ASTNode.hpp"
#include "adun/Parser/Command.hpp"
#include "adun/Parser/ExpressionNode.hpp"
#include "adun/Parser/Utils.hpp"

namespace adun::ast {

class SelectCommand final : public Command {
public:
  SelectCommand(std::vector<std::string> columns, std::string tableName,
                Ref<ExpressionNode> condition)
      : Command{ NodeKind::SelectCommand },
        m_Columns{ std::move(columns) },
        m_TableName{ std::move(tableName) },
        m_Condition{ std::move(condition) } {
  }

  auto execute(Database& db) -> Result override;

private:
  std::vector<std::string> m_Columns;
  std::string m_TableName;
  Ref<ExpressionNode> m_Condition;
};

} // namespace adun::ast
