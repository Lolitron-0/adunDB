#include "adun/Parser/Lexer.hpp"
#include "adun/Assert.hpp"
#include "adun/Parser/Token.hpp"
#include "adun/Parser/Utils.hpp"
#include <array>
#include <cctype>
#include <cul/cul.hpp>
#include <iterator>
#include <string_view>

namespace adun {

// clang-format off
#undef TOK
#undef KEYWORD
#undef PPTOK
#define KEYWORD(t) .Case(#t, TokenKind::KW_ ## t)
#define PPTOK(t) .Case("#" #t, TokenKind::PP_ ## t)
static constexpr cul::BiMap s_IdentifierMapping{
  [](auto selector) {
    return selector 
#include "adun/Parser/Tokens.def"
      ;
  }
};
#undef KEYWORD
#undef PPTOK
// clang-format on

// clang-format off
#undef TOK
#undef PUNCT
#define PUNCT(t, s) .Case(s, TokenKind::t)
static constexpr cul::BiMap s_PunctuatorMapping{
  [](auto selector) {
    return selector 
#include "adun/Parser/Tokens.def" 
      ;
  }
};
// clang-format on

#undef PUNCT
#define PUNCT(t, s) t,
namespace detail {
enum class CountPunctuators {
#include "adun/Parser/Tokens.def" // LParen, RParen, Equals, ...
  value
};
constexpr size_t CountPunctuators_v =
    static_cast<size_t>(CountPunctuators::value);
} // namespace detail

#undef PUNCT
#define PUNCT(t, s) s,
static constexpr std::array<std::string_view, detail::CountPunctuators_v>
    s_PunctuatorStrings = sortConstexpr(
        std::array<std::string_view, detail::CountPunctuators_v>{
#include "adun/Parser/Tokens.def" // "<=", "<", "&&", ...
        },
        [](const auto& a, const auto& b) {
          return a.length() > b.length();
        });
#undef PUNCT

static auto decodeEscapedChar(const SourceIt& pos) -> char {
  switch (*pos) {
  case 'a':
    return '\a';
  case 'b':
    return '\b';
  case 'f':
    return '\f';
  case 'n':
    return '\n';
  case 'r':
    return '\r';
  case 't':
    return '\t';
  case 'v':
    return '\v';
  case '0':
    return '\0';
  case '\\':
    return '\\';
  default:
    // emitWarning(pos, 1, "Invalid escape sequence, ignoring '\\'");
    return *pos;
  }
}

auto Lexer::lexIdentifier(SourceIt& pos) -> bool {
  auto start{ pos };
  auto ident{ consumeIdent(pos) };

  if (ident.empty()) {
    return false;
  }

  Token newTok{ TokenKind::Unknown, start, ident.size() };
  auto kind{ s_IdentifierMapping.FindByFirst(newTok.getStringView()) };
  newTok.setKind(
      kind.value_or(TokenKind::Identifier)); // either keyword or name
  m_Tokens->push_back(std::move(newTok));
  return true;
}

auto Lexer::lexNumericLiteral(SourceIt& pos) -> bool {
  auto start{ pos };
  if (!std::isdigit(*pos)) {
    return false;
  }

  size_t length{ 1 };
  ++pos;

  // we only support decimal integers for now
  while (std::isdigit(*pos)) {
    ++pos;
    ++length;
  }
  m_Tokens->emplace_back(TokenKind::NumericLiteral, start, length);
  m_Tokens->back().setLiteralValue(
      std::stoi(std::string{ start, start + length }));
  return true;
}

auto Lexer::lexStringLiteral(SourceIt& pos) -> bool {
  auto start{ pos };
  if (*pos != '"') {
    return false;
  }

  ++pos;
  std::string value{};

  while (*pos != '"' && *pos != '\n') {
    if (*pos == '\\') {
      ++pos; // consume backslash
      value += decodeEscapedChar(pos);
      ++pos;
      continue;
    }
    value += *pos;
    ++pos;
  }
  if (*pos == '"') {
    ++pos;
  } else {
    // emitError(start, value.length(), "Unclosed string literal");
    throw LexerFatalError();
    return false;
  }
  m_Tokens->emplace_back(TokenKind::StringLiteral, start,
                         value.length() + 2); // plus quotes
  m_Tokens->back().setLiteralValue(value);
  return true;
}

auto Lexer::lexPunctuator(SourceIt& pos) -> bool {
  // s_PunctuatorStrings is sorted by length descending, so we'll match
  // long punctuators first
  for (const auto& punct : s_PunctuatorStrings) {
    if (startsWith(pos, punct)) {
      auto pucntKindOpt{ s_PunctuatorMapping.FindByFirst(punct) };
      adun_assert(pucntKindOpt.has_value(),
                  "Punctuator mapping and list somehow doesn't match");
      m_Tokens->emplace_back(pucntKindOpt.value(), pos, punct.length());
      pos += punct.length();
      return true;
    }
  }

  return false;
}

Lexer::Lexer()
    : m_Tokens{ makeRef<TokenList>() } {
}

void Lexer::lex(const std::string& query) {
  SourceIt pos{ query.cbegin() };

  while (pos != query.cend()) {

    // Line comments
    if (startsWith(pos, "//")) {
      pos += 2;
      while (*pos != '\n') {
        ++pos;
      }
    }

    // Block comments
    if (startsWith(pos, "/*")) {
      pos += 2;
      auto blockCommentEnd{ query.find_first_of(
          "*/", std::distance(query.cbegin(), pos)) };
      if (blockCommentEnd == std::string::npos) {
        // emitError(pos, 2, "Unterminated block comment");
      }
      pos += static_cast<SourceIt::difference_type>(
          blockCommentEnd - std::distance(query.cbegin(), pos) + 2);
      continue;
    }

    // Newlines and spaces
    if (*pos == '\n' || *pos == ' ') {
      ++pos;
      continue;
    }

    // Numeric literals
    if (lexNumericLiteral(pos)) {
      continue;
    }

    // String literals
    if (lexStringLiteral(pos)) {
      continue;
    }

    // Identifiers
    if (lexIdentifier(pos)) {
      continue;
    }

    // Punctuators
    if (lexPunctuator(pos)) {
      continue;
    }

    // emitError(pos, 1, "Unknown token");
    throw LexerFatalError();
  }

  m_Tokens->emplace_back(TokenKind::Eof, pos, 0);
}

} // namespace adun
