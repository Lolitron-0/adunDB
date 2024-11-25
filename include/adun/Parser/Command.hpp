#pragma once
#include "adun/Parser/ASTNode.hpp"
#include "adun/Result.hpp"
#include <stdexcept>

namespace adun {
class Database;

class CommandException : public std::runtime_error {
public:
  using std::runtime_error::runtime_error;
};

namespace ast {

class Command : public Node {
public:
  using Node::Node;

  virtual auto execute(Database& db) -> Result = 0;
};

} // namespace ast
} // namespace adun
