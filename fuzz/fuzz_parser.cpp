#include "adun/Exceptions.hpp"
#include "adun/Parser/Lexer.hpp"
#include "adun/Parser/Parser.hpp"
#include <iostream>
#include <string>

auto main() -> int {
  std::string query;
  std::cin >> query;
  adun::Lexer lexer;

  try {
    lexer.lex(query);
    auto tokens{ lexer.getTokens() };

    adun::Parser parser{ tokens };
    auto ast{ parser.buildAST() };
  } catch (const adun::DatabaseException& e) {
    return 0;
  }
  return 0;
}
