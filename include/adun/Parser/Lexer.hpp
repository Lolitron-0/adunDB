#pragma once
#include "adun/Parser/Token.hpp"
#include "adun/Parser/Utils.hpp"
#include <list>
#include <stdexcept>

namespace adun {

using TokenList = std::list<Token>;

class LexerFatalError : public std::runtime_error {
public:
  LexerFatalError()
      : std::runtime_error{ "Lexer fatal failure. Aborting." } {
  }
};

class Lexer {
public:
  Lexer();
  void lex(const std::string& query);

  [[nodiscard]] auto getTokens() const -> Ref<TokenList> {
    return m_Tokens;
  }

private:
  auto lexNumericLiteral(SourceIt& pos) -> bool;
  auto lexIdentifier(SourceIt& pos) -> bool;
  auto lexStringLiteral(SourceIt& pos) -> bool;
  auto lexPunctuator(SourceIt& pos) -> bool;

  Ref<TokenList> m_Tokens;
};

} // namespace adun
