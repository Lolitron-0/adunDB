#pragma once
#include "adun/Parser/ASTNode.hpp"
#include "adun/Result.hpp"

namespace adun {
class Database;

namespace ast {

class Command : public Node {
public:
  using Node::Node;

  virtual auto execute(Database& db) -> Result = 0;
};

} // namespace ast
} // namespace adun
