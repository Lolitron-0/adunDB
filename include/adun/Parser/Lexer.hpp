#pragma once
#include "adun/Exceptions.hpp"
#include "adun/Parser/Token.hpp"
#include "adun/Parser/Utils.hpp"
#include <list>
#include <stdexcept>

namespace adun {

using TokenList = std::list<Token>;

class LexerFatalError : public DatabaseException {
public:
  using DatabaseException::DatabaseException;
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

  auto startsWith(const SourceIt& pos, std::string_view prefix) -> bool;
  void skipSpacesSince(SourceIt& pos);

  auto consumeIdent(SourceIt& pos) -> std::string_view;
  [[nodiscard]] auto nearEnd(const SourceIt& pos) const -> bool;

  Ref<TokenList> m_Tokens;
  size_t m_QueryLength;
  SourceIt m_QueryStart;
};

} // namespace adun
