#pragma once
#include "adun/Parser/ExpressionNode.hpp"

namespace adun::ast {

class ValueExpr final : public ExpressionNode {
public:
  explicit ValueExpr(Value value)
      : ExpressionNode{ NodeKind::NumberExpr, value.getType() },
        m_Value{ std::move(value) } {
  }

  [[nodiscard]] auto evaluate(
      const Row& /*row*/,
      const std::unordered_map<std::string, size_t>& /*columns*/) const
      -> Value override {
    return m_Value;
  }

  [[nodiscard]] auto getValue() const -> Value {
    return m_Value;
  }

private:
  Value m_Value;
};

} // namespace adun::ast
