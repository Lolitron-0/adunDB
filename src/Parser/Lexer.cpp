#include "adun/Parser/Lexer.hpp"
#include "adun/Assert.hpp"
#include "adun/Parser/Token.hpp"
#include "adun/Parser/Utils.hpp"
#include <array>
#include <cctype>
#include <cstdint>
#include <cul/cul.hpp>
#include <fmt/color.h>
#include <fmt/format.h>
#include <string_view>

namespace adun {

// clang-format off
#undef TOK
#undef KEYWORD
#define KEYWORD(t) .Case(#t, TokenKind::KW_ ## t)
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

template <typename... Args>
static void emitError(const SourceIt& around, size_t length,
                      const fmt::format_string<Args...>& msg,
                      Args&&... args) {
  std::string str{};
  str += fmt::format(fmt::fg(fmt::color::red), "Error around: '{}'\n",
                     std::string_view{ around, around + length });
  str += fmt::format(msg, std::forward<Args>(args)...);
  throw LexerFatalError{ str };
}

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
    emitError(pos, 1, "Invalid escape sequence, ignoring '\\'");
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
  auto kind{ s_IdentifierMapping.FindByFirstIgnoreCase(
      newTok.getStringView()) };
  newTok.setKind(
      kind.value_or(TokenKind::Identifier)); // either keyword or name
  m_Tokens->push_back(std::move(newTok));
  return true;
}

static auto byteArrayFromString(std::string_view str) -> ByteArray {
  ByteArray result;
  for (int32_t i{ static_cast<int32_t>(str.size() - 1) }; i >= 2;
       i -= 2) {
    uint8_t byte{};
    if (str[i] >= '0' && str[i] <= '9') {
      byte += str[i] - '0';
    } else if (tolower(str[i]) >= 'a' && tolower(str[i]) <= 'f') {
      byte += tolower(str[i]) - 'a' + 10;
    }

    if (tolower(str[i - 1]) == 'x') {
      byte += 0;
    } else if (str[i - 1] >= '0' && str[i - 1] <= '9') {
      byte += (str[i - 1] - '0') * 16;
    } else if (tolower(str[i - 1]) >= 'a' && tolower(str[i - 1]) <= 'f') {
      byte += (tolower(str[i - 1]) - 'a' + 10) * 16;
    }

    result.push_back(byte);
  }

  std::reverse(result.begin(), result.end());
  return result;
}

auto Lexer::lexNumericLiteral(SourceIt& pos) -> bool {
  auto start{ pos };
  if (!std::isdigit(*pos)) {
    return false;
  }

  size_t length{ 1 };
  auto firstDigit{ *pos };
  ++pos;
  if (firstDigit == '0' && (tolower(*pos) == 'x')) {
    ++pos;
    ++length;
    if (!std::isdigit(*pos) &&
        (tolower(*pos) < 'a' || tolower(*pos) > 'f')) {
      return false;
    }
    while (std::isdigit(*pos) ||
           (tolower(*pos) >= 'a' && tolower(*pos) <= 'f')) {
      ++pos;
      ++length;
    }
    m_Tokens->emplace_back(TokenKind::HexLiteral, start, length);
    auto literal{ byteArrayFromString(
        std::string_view{ start, start + length }) };
    m_Tokens->back().setLiteralValue(literal);
    return true;
  }

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
    emitError(start, value.length(), "Unclosed string literal");
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

    emitError(pos, 1, "Unknown token");
  }

  m_Tokens->emplace_back(TokenKind::Eof, pos, 0);
}

} // namespace adun
