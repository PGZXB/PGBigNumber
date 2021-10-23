#ifndef PGBIGNUMBER_INFIXEXPR_PARSER_H
#define PGBIGNUMBER_INFIXEXPR_PARSER_H

#include <stack>

#include "fwd.h"
#include "Token.h"
PGBN_INFIXEXPR_NAMESPACE_START

class ExprNode;
class NodePool;

class ExprParser {
    using Node = ExprNode;
public:
    ExprParser();
    
    template <typename S>
    ExprParser(S & stream, const Hooks & hooks, NodePool * pNodePool);

    template <typename S>
    void init(S & stream, const Hooks & hooks, NodePool * pNodePool);

    ExprParser(const ExprParser &) = delete;
    ExprParser(ExprParser &&) = delete;
    ExprParser & operator=(const ExprParser &) = delete;
    ExprParser & operator=(ExprParser &&) = delete;

    Node * parse();

private:
    Node * parseExpr();
    Node * parseConditional();
    Node * parseBitOr();
    Node * parseBitXor();
    Node * parseBitAnd();
    Node * parseEqu();
    Node * parseCmp();
    Node * parseShift();
    Node * parseAddSub();
    Node * parseMulDivMod();
    Node * parseUnaryOp();
    Node * parsePostfixOp();
    Node * parsePrim();
    Node * parseFuncCall(SymbolTable::Symbol *);

    bool consumeToken(TokenType type);
    Token & nextToken();
    bool hasNextToken() const;
    void badToken(TokenType type, const StringArg & msg) const;

private:
    std::vector<Token> m_tokens;
    SizeType m_currTokenIndex = 0;
    NodePool * m_pNodePool = nullptr;
    mutable Enum m_errCode = ErrCode::SUCCESS;
};

template <typename S>
inline ExprParser::ExprParser(
    S & stream, const Hooks & hooks, NodePool * pNodePool
) : m_tokens(tokenizer(stream, hooks)), m_pNodePool(pNodePool) {

}

template <typename S>
void ExprParser::init(S & stream, const Hooks & hooks, NodePool * pNodePool) {
    m_tokens = tokenizer(stream, hooks);
    m_currTokenIndex = 0;
    m_pNodePool = pNodePool;
}

PGBN_INFIXEXPR_NAMESPACE_END
#endif // !PGBIGNUMBER_INFIXEXPR_PARSER_H