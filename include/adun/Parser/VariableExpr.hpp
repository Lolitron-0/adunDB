#pragma once
#include "adun/Exceptions.hpp"
#include "adun/Parser/ASTNode.hpp"
#include "adun/Parser/ExpressionNode.hpp"
#include "adun/Row.hpp"
#include <utility>

namespace adun::ast {

class VariableExpr final : public ExpressionNode {
public:
  explicit VariableExpr(std::string name)
      : ExpressionNode{ NodeKind::VariableExpr },
        m_Name{ std::move(name) } {
  }

  [[nodiscard]] auto evaluate(
      const Row& row,
      const std::unordered_map<std::string, size_t>& columns) const
      -> Value override {
    if (!columns.contains(m_Name)) {
      throw NoSuchColumnException{ m_Name };
    }
    return row.get(columns.at(m_Name));
  }

  [[nodiscard]] auto getVarName() const -> std::string {
    return m_Name;
  }

  friend class QueryASTRoot;

private:
  std::string m_Name;
};

} // namespace adun::ast
