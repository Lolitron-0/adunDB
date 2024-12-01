#pragma once
#include "adun/Parser/Command.hpp"
#include "adun/Parser/ExpressionNode.hpp"
#include "adun/Parser/Utils.hpp"

namespace adun::ast {

class DeleteCommand final : public Command {
public:
  DeleteCommand(std::string tableName, Ref<ExpressionNode> condition)
      : Command{ NodeKind::DeleteCommand },
        m_TableName{ std::move(tableName) },
        m_Condition{ std::move(condition) } {
  }

  auto execute(Database& db) -> Result override;

private:
  std::string m_TableName;
  Ref<ExpressionNode> m_Condition;
};

} // namespace adun::ast
