#include "adun/Database.hpp"
#include "adun/Parser/Lexer.hpp"
#include "adun/Parser/Parser.hpp"

namespace adun {

auto Database::execute(const std::string& queryString) -> Result {
  Lexer lexer;
  lexer.lex(queryString);
  auto tokens{ lexer.getTokens() };

  Parser parser{ tokens };
  auto query{ parser.buildAST() };

  return query->execute(*this);
}

} // namespace adun
