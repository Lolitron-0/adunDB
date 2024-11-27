#pragma once
#include "adun/Exceptions.hpp"
#include "adun/Parser/ExpressionNode.hpp"
#include "adun/Parser/Token.hpp"
#include "adun/Value.hpp"

namespace adun::ast {

class UnaryOpException : public DatabaseException {
  using DatabaseException::DatabaseException;
};

class UnaryOpExpr final : public ExpressionNode {
public:
  UnaryOpExpr(Ref<ExpressionNode> operand, TokenKind op);

  [[nodiscard]] auto evaluate(
      const Row& row,
      const std::unordered_map<std::string, size_t>& columns) const
      -> Value override {
    Value operand = m_Operand->evaluate(row, columns);

    switch (m_Op) {
    case TokenKind::Pipe:
      if (operand.getType() != ValueType::String) {
        throw UnaryOpException{ "Unary operator of incompatible type" };
      }
      return static_cast<int32_t>(operand.get<std::string>().size());
    case TokenKind::Minus:
      return -operand;
    default:
      adun_assert_nomsg(false);
    }
    return {};
  }

  [[nodiscard]] auto getOp() const -> TokenKind;
  [[nodiscard]] auto getOperand() -> Ref<ExpressionNode>;

private:
  TokenKind m_Op;
  Ref<ExpressionNode> m_Operand;
};

} // namespace adun::ast
