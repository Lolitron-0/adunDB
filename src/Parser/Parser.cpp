#include "adun/Parser/Parser.hpp"
#include "adun/Assert.hpp"
#include "adun/Column.hpp"
#include "adun/Parser/ASTNode.hpp"
#include "adun/Parser/BinOpExpr.hpp"
#include "adun/Parser/CreateCommand.hpp"
#include "adun/Parser/ExpressionNode.hpp"
#include "adun/Parser/Token.hpp"
#include "adun/Parser/UnaryOpExpr.hpp"
#include "adun/Parser/UpdateCommand.hpp"
#include "adun/Parser/Utils.hpp"
#include "adun/Parser/ValueExpr.hpp"
#include "adun/Parser/VariableExpr.hpp"
#include "adun/Value.hpp"
#include <fmt/base.h>
#include <fmt/color.h>
#include <frozen/unordered_map.h>

namespace adun {

static constexpr auto s_BinopPrecedence{
  frozen::make_unordered_map<TokenKind, int32_t>({
      { TokenKind::Plus, 20 },
      { TokenKind::Minus, 20 },
      { TokenKind::Star, 40 },
      { TokenKind::Div, 40 },
      { TokenKind::Less, 10 },
      { TokenKind::LessEqual, 10 },
      { TokenKind::Greater, 10 },
      { TokenKind::GreaterEqual, 10 },
      { TokenKind::Equals, 10 },
      { TokenKind::Or, 9 },
      { TokenKind::And, 9 },
      { TokenKind::Xor, 9 },
  })
};

template <typename... Args>
static void emitError(const Token& around,
                      const fmt::format_string<Args...>& msg,
                      Args&&... args) {
  std::string str{};
  str += fmt::format(fmt::fg(fmt::color::red), "Error around: '{}'\n",
                     around.getStringView());
  str += fmt::format(msg, std::forward<Args>(args)...);
  throw ParserException{ str };
}

auto Parser::buildAST() -> Ref<ast::Command> {
  switch (curTok().getKind()) {
  case TokenKind::KW_create:
    m_ASTRoot = parseCreateCommand();
    break;
  case TokenKind::KW_insert:
    m_ASTRoot = parseInsertCommand();
    break;
  case TokenKind::KW_select:
    m_ASTRoot = parseSelectCommand();
    break;
  case TokenKind::KW_update:
    m_ASTRoot = parseUpdateCommand();
    break;
  default:
    emitError(curTok(), "Expected command name");
  }
  return m_ASTRoot;
}

auto Parser::parseCreateCommand() -> Unique<ast::CreateCommand> {
  adun_assert(curTok().is(TokenKind::KW_create), "Expected 'CREATE'");
  consumeToken();
  expectConsume(TokenKind::KW_table);

  expect(TokenKind::Identifier);
  std::string tableName{ curTok().getStringView() };
  consumeToken();

  expectConsume(TokenKind::LParen);

  auto schemeTok{ curTok() };
  auto scheme{ parseScheme() };
  if (scheme.empty()) {
    emitError(schemeTok, "Failed to parse table scheme");
  }

  // RParen already consumed by parseScheme
  expectConsumeEnd();
  return makeUnique<ast::CreateCommand>(std::move(tableName),
                                        std::move(scheme));
}

auto Parser::parseInsertCommand() -> Unique<ast::InsertCommand> {
  adun_assert(curTok().is(TokenKind::KW_insert), "Expected 'INSERT'");
  consumeToken();
  expectConsume(TokenKind::LParen);

  std::vector<std::pair<std::string, Value>> assignments;
  while (curTok().isNot(TokenKind::RParen)) {
    expect(TokenKind::Identifier);
    std::string columnName{ curTok().getStringView() };
    consumeToken();

    expectConsume(TokenKind::Equals);

    auto value{ parseValueExpr()->getValue() };

    if (curTok().is(TokenKind::Comma)) {
      consumeToken();
    } else if (curTok().is(TokenKind::RParen)) { // will be consumed later
    } else {
      emitError(curTok(), "Expected ','");
    }

    assignments.emplace_back(columnName, value);
  }
  consumeToken();

  expectConsume(TokenKind::KW_into);

  expect(TokenKind::Identifier);
  std::string tableName{ curTok().getStringView() };
  consumeToken();

  expectConsumeEnd();

  return makeUnique<ast::InsertCommand>(std::move(tableName),
                                        std::move(assignments));
}

auto Parser::parseSelectCommand() -> Unique<ast::SelectCommand> {
  adun_assert(curTok().is(TokenKind::KW_select), "Expected 'SELECT'");
  consumeToken();

  auto columns{ parseColumnNames() };

  expectConsume(TokenKind::KW_from);

  expect(TokenKind::Identifier);
  std::string tableName{ curTok().getStringView() };
  consumeToken();

  Unique<ast::ExpressionNode> cond;
  if (curTok().isNot(TokenKind::KW_where)) {
    cond = makeUnique<ast::ValueExpr>(true);
  } else {
    consumeToken();
    cond = parseExpression();
  }
  expectConsumeEnd();

  return makeUnique<ast::SelectCommand>(
      std::move(columns), std::move(tableName), std::move(cond));
}

auto Parser::parseUpdateCommand() -> Unique<ast::UpdateCommand> {
  adun_assert(curTok().is(TokenKind::KW_update), "Expected 'UPDATE'");
  consumeToken();

  expect(TokenKind::Identifier);
  std::string tableName{ curTok().getStringView() };
  consumeToken();

  expectConsume(TokenKind::KW_set);
  expectConsume(TokenKind::LParen);

  std::vector<std::pair<std::string, Ref<ast::ExpressionNode>>>
      assignments;
  while (curTok().isNot(TokenKind::RParen)) {
    expect(TokenKind::Identifier);
    std::string columnName{ curTok().getStringView() };
    consumeToken();

    expectConsume(TokenKind::Equals);

    Ref<ast::ExpressionNode> expr{ parseExpression() };

    if (curTok().is(TokenKind::Comma)) {
      consumeToken();
    } else if (curTok().is(TokenKind::RParen)) {
      // will be consumed later
    } else {
      emitError(curTok(), "Expected ','");
    }

    assignments.emplace_back(columnName, std::move(expr));
  }
  consumeToken();

  expectConsume(TokenKind::KW_where);

  Ref<ast::ExpressionNode> cond{ parseExpression() };

  expectConsumeEnd();
  return makeUnique<ast::UpdateCommand>(
      std::move(tableName), std::move(assignments), std::move(cond));
}

auto Parser::parseValueExpr() -> Unique<ast::ValueExpr> {
  Value value;
  Unique<ast::ExpressionNode> compoundResult;
  switch (curTok().getKind()) {
  case TokenKind::NumericLiteral:
    value = curTok().getLiteralValue<int32_t>();
    break;
  case TokenKind::StringLiteral:
    value = curTok().getLiteralValue<std::string>();
    break;
  case TokenKind::KW_true:
    value = true;
    break;
  case TokenKind::KW_false:
    value = false;
    break;
  case TokenKind::HexLiteral:
    value = curTok().getLiteralValue<ByteArray>();
    break;
  case TokenKind::KW_null:
    value = curTok().getLiteralValue();
    break;
  default:
    emitError(curTok(), "Expected value expression");
    return invalidNode();
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
  }

  if (curTok().isNot(TokenKind::RParen)) {
    emitError(curTok(), "Expected ')'");
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

  return invalidNode();
}

auto Parser::parseCompoundExpression()
    -> std::unique_ptr<ast::ExpressionNode> {
  Unique<ast::ExpressionNode> compoundResult;
  switch (curTok().getKind()) {
  case TokenKind::Identifier:
    return parseIdentifierExpr();
  case TokenKind::LParen:
    return parseParenExpr();
  case TokenKind::Minus:
    consumeToken();
    return makeUnique<ast::UnaryOpExpr>(parseValueExpr(),
                                        TokenKind::Minus);
  case TokenKind::Pipe:
    consumeToken();
    compoundResult =
        makeUnique<ast::UnaryOpExpr>(parseExpression(), TokenKind::Pipe);
    if (curTok().isNot(TokenKind::Pipe)) {
      emitError(curTok(), "Expected closing '|'");
      return invalidNode();
    }
    consumeToken();
    return compoundResult;
  default:
    return parseValueExpr();
  }
  return invalidNode();
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
  case TokenKind::KW_integer:
    consumeToken();
    return ValueType::Integer;
  case TokenKind::KW_string:
    consumeToken();
    return ValueType::String;
  case TokenKind::KW_bool:
    consumeToken();
    return ValueType::Boolean;
  case TokenKind::KW_byte:
    consumeToken();
    return ValueType::Binary;
  default:
    return ValueType::None;
  }
}

auto Parser::parseScheme() -> Table::Scheme {
  Table::Scheme scheme;
  while (curTok().isNot(TokenKind::RParen)) {
    expect(TokenKind::Identifier);
    std::string columnName{ curTok().getStringView() };
    if (scheme.contains(columnName)) {
      emitError(curTok(), "Duplicate column name");
      return {};
    }
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
      case TokenKind::KW_autoincrement:
        columnModifiers |= Column::Modifier::AutoIncrement;
        consumeToken();
        break;
      case TokenKind::KW_unique:
        columnModifiers |= Column::Modifier::Unique;
        consumeToken();
        break;
      case TokenKind::KW_default:
        columnModifiers |= Column::Modifier::HasDefault;
        defaultKWTok = curTok();
        consumeToken();
        expectConsume(TokenKind::LParen);
        columnValue = parseValueExpr()->getValue();
        if (columnValue.isEmpty() || columnValue.isNull()) {
          emitError(defaultKWTok, "Expected non-empty value");
          return {};
        }
        if (columnValue.getType() != columnType) {
          emitError(defaultKWTok, "Default value type mismatch");
          return {};
        }
        expectConsume(TokenKind::RParen);
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

auto Parser::parseColumnNames() -> std::vector<std::string> {
  std::vector<std::string> columns;

  if (curTok().is(TokenKind::Star)) {
    consumeToken();
    return columns;
  }

  if (!curTok().is(TokenKind::Identifier)) {
    emitError(curTok(), "Expected column name");
  }
  columns.emplace_back(curTok().getStringView());
  consumeToken();
  while (curTok().is(TokenKind::Comma)) {
    consumeToken();
    if (!curTok().is(TokenKind::Identifier)) {
      emitError(curTok(), "Expected column name");
    }
    columns.emplace_back(curTok().getStringView());
    consumeToken();
  }

  return columns;
}
void Parser::expect(TokenKind kind) {
  if (curTok().isNot(kind)) {
    emitError(curTok(), "Expected: '{}'",
              Token::TokenNameMapping.Find(kind).value());
  }
}
void Parser::expectConsume(TokenKind kind) {
  expect(kind);
  consumeToken();
}
void Parser::expectConsumeEnd() {
  if (!curTok().isOneOf(TokenKind::Semicolon, TokenKind::Eof)) {
    emitError(curTok(), "Expected end of query");
  }
  consumeToken();
}
} // namespace adun
