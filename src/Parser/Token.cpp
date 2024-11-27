#include "adun/Parser/Token.hpp"
#include "adun/Assert.hpp"

namespace adun {

Token::Token(TokenKind kind, SourceIt loc, size_t length)
    : m_Kind{ kind },
      m_Length{ length },
      m_Loc{ loc } {
}

auto Token::getKind() const -> TokenKind {
  return m_Kind;
}

void Token::setKind(TokenKind kind) {
  m_Kind = kind;
}

auto Token::getStringView() const -> std::string_view {
  if (is(TokenKind::Eof)) {
    return "EOF";
  }
  return std::string_view{ m_Loc, m_Loc + m_Length };
}

auto Token::getLength() const -> size_t {
  return m_Length;
}

} // namespace adun
