#include "adun/Parser/ExpressionNode.hpp"
#include "adun/Value.hpp"

namespace adun::ast {

ExpressionNode::ExpressionNode(NodeKind kind)
    : ExpressionNode{ kind, ValueType::None } {
}

ExpressionNode::ExpressionNode(NodeKind kind, ValueType type)
    : Node{ kind },
      m_Type{ type } {
}

auto ExpressionNode::isTypeResolved() const -> bool {
  return m_Type != ValueType::None;
}

auto ExpressionNode::getType() const -> ValueType {
  return m_Type;
}

void ExpressionNode::setType(ValueType type) {
  m_Type = type;
}

} // namespace adun
