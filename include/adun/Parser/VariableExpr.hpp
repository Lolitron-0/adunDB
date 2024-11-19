#pragma once
#include "adun/Parser/ASTNode.hpp"
#include "adun/Parser/ExpressionNode.hpp"
#include <utility>

namespace adun::ast {

class VariableExpr final : public ExpressionNode {
public:
  explicit VariableExpr(std::string name)
      : ExpressionNode{ NodeKind::VariableExpr },
        m_Name{ std::move(name) } {
  }

  [[nodiscard]] auto getVarName() const -> std::string {
    return m_Name;
  }

  friend class QueryASTRoot;

private:
  std::string m_Name;
};

} // namespace adun::ast
