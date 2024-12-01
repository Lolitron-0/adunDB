#pragma once
#include "adun/Parser/Command.hpp"
#include "adun/Parser/ExpressionNode.hpp"
#include "adun/Parser/Utils.hpp"

namespace adun::ast {

class UpdateCommand final : public Command {
public:
  UpdateCommand(
      std::string tableName,
      std::vector<std::pair<std::string, Ref<ExpressionNode>>> values,
      Ref<ExpressionNode> condition)
      : Command{ NodeKind::UpdateCommand },
        m_TableName{ std::move(tableName) },
        m_Values{ std::move(values) },
        m_Condition{ std::move(condition) } {
  }

  auto execute(Database& db) -> Result override;

private:
  std::string m_TableName;
  std::vector<std::pair<std::string, Ref<ExpressionNode>>> m_Values;
  Ref<ExpressionNode> m_Condition;
};

} // namespace adun::ast
