#pragma once
#include "adun/Parser/Utils.hpp"
#include <string_view>
#include <utility>
#include <vector>

namespace adun::ast {

class Command;

namespace internal {
template <typename T>
void dump(int depth, std::string_view name, T value);
} // namespace internal

/// For grammar description of each node refer to corresponding method in
/// Parser
enum class NodeKind {
  NumberExpr,
  VariableExpr,
  StringExpr,
  BinOpExpr,
  CreateCommand,
  QueryASTRoot,
  NUM_NODES
};

class Node {
protected:
  explicit Node(NodeKind kind)
      : m_Kind{ kind } {
  }

public:
  virtual ~Node() = default;

  [[nodiscard]] auto getKind() const -> NodeKind {
    return m_Kind;
  }

private:
  NodeKind m_Kind;
};

class Statement : public Node {
protected:
  explicit Statement(NodeKind kind)
      : Node{ kind } {
  }
};

} // namespace adun::ast