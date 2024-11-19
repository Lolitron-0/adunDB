#include "adun/Parser/Parser.hpp"
#include "adun/Assert.hpp"
#include "adun/Column.hpp"
#include "adun/Parser/ASTNode.hpp"
#include "adun/Parser/BinOpExpr.hpp"
#include "adun/Parser/CreateCommand.hpp"
#include "adun/Parser/Token.hpp"
#include "adun/Parser/Utils.hpp"
#include "adun/Parser/VariableExpr.hpp"
#include "adun/Value.hpp"
#include <fmt/base.h>
#include <fmt/color.h>
#include <frozen/unordered_map.h>

namespace adun {

static constexpr auto s_BinopPrecedence{
  frozen::make_unordered_map<TokenKind, int32_t>(
      { { TokenKind::Plus, 20 },
        { TokenKind::Minus, 20 },
        { TokenKind::Star, 40 },
        { TokenKind::Div, 40 },
        { TokenKind::Less, 10 } })
};

template <typename... Args>
static void emitError(Token around,
                      const fmt::format_string<Args...>& msg,
                      Args&&... args) {
  fmt::print(stderr, fmt::fg(fmt::color::red), "Error around: '{}'\n",
             around.getStringView());
  fmt::println(stderr, msg, std::forward<Args>(args)...);
}

auto Parser::buildAST() -> Ref<ast::Command> {
  switch (curTok().getKind()) {
  case TokenKind::KW_CREATE:
    m_ASTRoot = parseCreateCommand();
    break;
  default:
    m_ASTRoot = invalidNode();
  }
  return m_ASTRoot;
}

auto Parser::parseCreateCommand() -> Unique<ast::CreateCommand> {
  adun_assert(curTok().is(TokenKind::KW_CREATE), "Expected 'CREATE'");
  consumeToken();
  if (curTok().isNot(TokenKind::KW_TABLE)) {
    emitError(curTok(), "Expected 'TABLE'");
    return invalidNode();
  }
  consumeToken();

  if (curTok().isNot(TokenKind::Identifier)) {
    emitError(curTok(), "Expected table name");
    return invalidNode();
  }
  std::string tableName{ curTok().getStringView() };
  consumeToken();

  if (curTok().isNot(TokenKind::LParen)) {
    emitError(curTok(), "Expected '('");
    return invalidNode();
  }
  consumeToken();

  auto schemeTok{ curTok() };
  auto scheme{ parseScheme() };
  if (scheme.empty()) {
    emitError(schemeTok, "Failed to parse table scheme");
    return invalidNode();
  }

  if (!curTok().isOneOf(TokenKind::Semicolon, TokenKind::Eof)) {
    emitError(curTok(), "Expected end of query");
    return invalidNode();
  }
  consumeToken();
  return makeUnique<ast::CreateCommand>(tableName, scheme);
}

auto Parser::parseValueExpr() -> Unique<ast::ValueExpr> {
  Value value;
  switch (curTok().getKind()) {
  case TokenKind::NumericLiteral:
    value = curTok().getLiteralValue<int32_t>();
    break;
  case TokenKind::StringLiteral:
    value = curTok().getLiteralValue<std::string>();
    break;
  case TokenKind::BoolLiteral:
    value = curTok().getLiteralValue<bool>();
    break;
  case TokenKind::HexLiteral:
    value = curTok().getLiteralValue<ByteArray>();
    break;
  default:
    break;
  }
  auto token{ curTok() };
  consumeToken();
  return makeUnique<ast::ValueExpr>(value);
}

auto Parser::parseParenExpr() -> Unique<ast::ExpressionNode> {
  adun_assert(curTok().is(TokenKind::LParen), "Expected '('");
  consumeToken();

  auto result{ parseExpression() };
  if (!result) {
    return invalidNode();
  }

  if (curTok().isNot(TokenKind::RParen)) {
    // emitError(curTok(), "Expected ')'");
    return invalidNode();
  }

  consumeToken();
  return result;
}

auto Parser::parseIdentifierExpr() -> Unique<ast::ExpressionNode> {
  adun_assert(curTok().is(TokenKind::Identifier), "Expected identifier");
  auto identifierTok{ curTok() };

  consumeToken();

  if (curTok().isNot(TokenKind::LParen)) {
    // it's a variable
    return makeUnique<ast::VariableExpr>(
        std::string{ identifierTok.getStringView() });
  }

  /// @todo parse function call
  return invalidNode();
}

auto Parser::parseCompoundExpression()
    -> std::unique_ptr<ast::ExpressionNode> {
  switch (curTok().getKind()) {
  case TokenKind::Identifier:
    return parseIdentifierExpr();
  case TokenKind::LParen:
    return parseParenExpr();
  case TokenKind::NumericLiteral:
    return parseValueExpr();
  default:
    // emitError(curTok(), "Expected value expression");
    return invalidNode();
  }
}

auto Parser::parseExpression() -> std::unique_ptr<ast::ExpressionNode> {
  auto lhs{ parseCompoundExpression() };
  if (!lhs) {
    return invalidNode();
  }
  return parseBinOpRhs(std::move(lhs));
}

static auto getTokPrecedence(const Token& tok) -> int32_t {
  if (s_BinopPrecedence.contains(tok.getKind())) {
    return s_BinopPrecedence.at(tok.getKind());
  }
  return -1;
}

auto Parser::parseBinOpRhs(std::unique_ptr<ast::ExpressionNode> lhs,
                           int32_t prevPrecedence)
    -> std::unique_ptr<ast::ExpressionNode> {
  while (true) {
    // precedence or -1 if not a binop
    auto tokPrecedence{ getTokPrecedence(curTok()) };
    if (tokPrecedence < prevPrecedence) {
      return lhs;
    }

    auto binOp{ curTok() };
    consumeToken();
    auto rhs{ parseCompoundExpression() };
    if (!rhs) {
      return invalidNode();
    }

    // now: lhs binOp rhs unparsed

    auto nextPrecedence{ getTokPrecedence(curTok()) };
    // if associates to the right: lhs binOp (rhs lookahead unparsed)
    if (tokPrecedence < nextPrecedence) {
      rhs = parseBinOpRhs(std::move(rhs), tokPrecedence + 1);
      if (!rhs) {
        return invalidNode();
      }
    }

    // now: (lhs binOp rhs) lookahead unparsed
    lhs = makeUnique<ast::BinOpExpr>(std::move(lhs), std::move(rhs),
                                     binOp.getKind());
  }
}

auto Parser::lookahead(uint32_t offset) const -> const Token& {
  auto iter{ m_CurTokIter };
  std::advance(iter, offset);
  return *iter;
}

auto Parser::parseTypename() -> ValueType {
  switch (curTok().getKind()) {
  case TokenKind::KW_INTEGER:
    consumeToken();
    return ValueType::Integer;
  case TokenKind::KW_STRING:
    consumeToken();
    return ValueType::String;
  case TokenKind::KW_BOOL:
    consumeToken();
    return ValueType::Boolean;
  case TokenKind::KW_BYTE:
    consumeToken();
    return ValueType::Binary;
  default:
    return ValueType::None;
  }
}

auto Parser::parseScheme() -> Table::Header {
  Table::Header scheme;
  while (curTok().isNot(TokenKind::RParen)) {
    if (curTok().isNot(TokenKind::Identifier)) {
      emitError(curTok(), "Expected column name");
      return {};
    }
    std::string columnName{ curTok().getStringView() };
    consumeToken();

    auto columnType{ parseTypename() };
    if (columnType == ValueType::None) {
      emitError(curTok(), "Expected column type");
      return {};
    }
    Value columnValue{ columnType };

    Token defaultKWTok{ curTok() };
    Column::ModifierFlags columnModifiers{ Column::Modifier::None };
    while (curTok().isNot(TokenKind::Comma) &&
           curTok().isNot(TokenKind::RParen)) {
      switch (curTok().getKind()) {
      case TokenKind::KW_AUTOINCREMENT:
        columnModifiers |= Column::Modifier::AutoIncrement;
        consumeToken();
        break;
      case TokenKind::KW_UNIQUE:
        columnModifiers |= Column::Modifier::Unique;
        consumeToken();
        break;
      case TokenKind::KW_DEFAULT:
        columnModifiers |= Column::Modifier::HasDefault;
        defaultKWTok = curTok();
        consumeToken();
        if (curTok().isNot(TokenKind::LParen)) {
          emitError(curTok(), "Expected '('");
          return {};
        }
        consumeToken();
        columnValue = parseValueExpr()->getValue();
        if (columnValue.isEmpty() || columnValue.isNull()) {
          emitError(defaultKWTok, "Expected non-empty value");
          return {};
        }
        if (columnValue.getType() != columnType) {
          emitError(defaultKWTok, "Default value type mismatch");
          return {};
        }
        if (curTok().isNot(TokenKind::RParen)) {
          emitError(curTok(), "Expected ')'");
          return {};
        }
        consumeToken();
        break;
      default:
        emitError(curTok(), "Expected column modifier");
        return {};
      }
    }
    // paren will be consumed bu outer while
    if (curTok().is(TokenKind::Comma)) {
      consumeToken();
    }

    scheme[columnName] = Column{ columnValue, columnModifiers };
  }
  consumeToken();
  return scheme;
}

} // namespace adun
