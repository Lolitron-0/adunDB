#pragma once
#include "adun/Exceptions.hpp"
#include "adun/Parser/ASTNode.hpp"
#include "adun/Parser/CreateCommand.hpp"
#include "adun/Parser/ExpressionNode.hpp"
#include "adun/Parser/InsertCommand.hpp"
#include "adun/Parser/Lexer.hpp"
#include "adun/Parser/SelectCommand.hpp"
#include "adun/Parser/UpdateCommand.hpp"
#include "adun/Parser/ValueExpr.hpp"
#include "adun/Value.hpp"
#include <cstddef>

namespace adun {

class Parser {
public:
  explicit Parser(const Ref<TokenList>& tokenList)
      : m_Tokens{ tokenList },
        m_CurTokIter{ tokenList->begin() },
        m_ASTRoot{ nullptr } {
  }

  auto buildAST() -> Ref<ast::Command>;

  [[nodiscard]] auto isASTInvalid() const -> bool {
    return m_ASTInvalid;
  }

private:
  auto parseCreateCommand() -> Unique<ast::CreateCommand>;
  auto parseInsertCommand() -> Unique<ast::InsertCommand>;
  auto parseSelectCommand() -> Unique<ast::SelectCommand>;
  auto parseUpdateCommand() -> Unique<ast::UpdateCommand>;
  auto parseValueExpr() -> Unique<ast::ValueExpr>;
  auto parseParenExpr() -> Unique<ast::ExpressionNode>;
  auto parseIdentifierExpr() -> Unique<ast::ExpressionNode>;
  auto parseCompoundExpression() -> Unique<ast::ExpressionNode>;
  auto parseExpression() -> Unique<ast::ExpressionNode>;
  auto parseBinOpRhs(Unique<ast::ExpressionNode> lhs,
                     int32_t prevPrecedence = 0)
      -> Unique<ast::ExpressionNode>;
  auto parseTypename() -> ValueType;
  auto parseScheme() -> Table::Scheme;
  auto parseColumnNames() -> std::vector<std::string>;

  [[nodiscard]] auto isFunctionDecl() const -> bool;

  [[nodiscard]] auto lookahead(uint32_t offset) const -> const Token&;

  void expect(TokenKind kind);
  void expectConsume(TokenKind kind);
  void expectConsumeEnd();

  [[nodiscard]] inline auto curTok() const -> const Token& {
    return *m_CurTokIter;
  }

  [[nodiscard]] auto invalidNode() -> std::nullptr_t {
    m_ASTInvalid = true;
    return nullptr;
  }

  inline void consumeToken() {
    m_CurTokIter++;
  }

  Ref<TokenList> m_Tokens;
  TokenList::const_iterator m_CurTokIter;
  Ref<ast::Command> m_ASTRoot;
  bool m_ASTInvalid{ false };
};

} // namespace adun
