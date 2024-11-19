#pragma once
#include "adun/Parser/ASTNode.hpp"
#include "adun/Value.hpp"

namespace adun::ast {

class ExpressionNode : public Node {
protected:
  // in case expression type is not yet known
  explicit ExpressionNode(NodeKind kind);
  ExpressionNode(NodeKind kind, ValueType type);

public:
  [[nodiscard]] auto isTypeResolved() const -> bool;

  [[nodiscard]] auto getType() const -> ValueType;

  void setType(ValueType type);

protected:
  ValueType m_Type;
};

} // namespace adun::ast
