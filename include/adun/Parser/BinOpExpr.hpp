#pragma once
#include "adun/Parser/ExpressionNode.hpp"
#include "adun/Parser/Token.hpp"

namespace adun::ast {

class BinOpExpr final : public ExpressionNode {
public:
  BinOpExpr(Ref<ExpressionNode> lhs, Ref<ExpressionNode> rhs,
            TokenKind op);

  [[nodiscard]] auto getOp() const -> TokenKind;
  [[nodiscard]] auto getLhs() -> Ref<ExpressionNode>;
  [[nodiscard]] auto getRhs() -> Ref<ExpressionNode>;
  [[nodiscard]] constexpr auto isArithmetic() const -> bool {
    /// @todo bitwise operators
    return m_Op == TokenKind::Plus || m_Op == TokenKind::Minus ||
           m_Op == TokenKind::Star || m_Op == TokenKind::Div ||
           m_Op == TokenKind::Amp || m_Op == TokenKind::Pipe;
  }

private:
  TokenKind m_Op;
  Ref<ExpressionNode> m_Lhs;
  Ref<ExpressionNode> m_Rhs;
};

} // namespace adun::ast
