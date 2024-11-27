#pragma once
#include "adun/Assert.hpp"
#include "adun/Exceptions.hpp"
#include "adun/Parser/ExpressionNode.hpp"
#include "adun/Parser/Token.hpp"

namespace adun::ast {

class BinOpException : public DatabaseException {
  using DatabaseException::DatabaseException;
};

class BinOpExpr final : public ExpressionNode {
public:
  BinOpExpr(Ref<ExpressionNode> lhs, Ref<ExpressionNode> rhs,
            TokenKind op);

  [[nodiscard]] auto evaluate(
      const Row& row,
      const std::unordered_map<std::string, size_t>& columns) const
      -> Value override {
    Value lhs = m_Lhs->evaluate(row, columns);
    Value rhs = m_Rhs->evaluate(row, columns);
    if (lhs.getType() != rhs.getType()) {
      throw BinOpException{ "Binary operation of incompatible types: " +
                            lhs.toString() + ", " + rhs.toString() };
    }

    switch (m_Op) {
    case TokenKind::Plus:
      return lhs + rhs;
    case TokenKind::Minus:
      return lhs - rhs;
    case TokenKind::Star:
      return lhs * rhs;
    case TokenKind::Div:
      return lhs / rhs;
    case TokenKind::Mod:
      return lhs % rhs;
    case TokenKind::Equals:
      return lhs == rhs;
    case TokenKind::Less:
      return lhs < rhs;
    case TokenKind::LessEqual:
      return lhs <= rhs;
    case TokenKind::Greater:
      return lhs > rhs;
    case TokenKind::GreaterEqual:
      return lhs >= rhs;
    case TokenKind::And:
      return lhs && rhs;
    case TokenKind::Or:
      return lhs || rhs;
    case TokenKind::Xor:
      return lhs ^ rhs;
    default:
      adun_assert_nomsg(false);
    }
    return {};
  }

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
