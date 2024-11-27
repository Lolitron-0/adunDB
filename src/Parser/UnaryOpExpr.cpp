#include "adun/Parser/UnaryOpExpr.hpp"

namespace adun::ast {

UnaryOpExpr::UnaryOpExpr(Ref<ExpressionNode> operand, TokenKind op)
    : ExpressionNode{ NodeKind::UnaryOpExpr },
      m_Op{ op },
      m_Operand{ std::move(operand) } {
}

auto UnaryOpExpr::getOp() const -> TokenKind {
  return m_Op;
}

auto UnaryOpExpr::getOperand() -> Ref<ExpressionNode> {
  return m_Operand;
}

} // namespace adun::ast
