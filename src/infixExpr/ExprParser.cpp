#include "ExprParser.h"
#include "ExprTree.h"

using namespace pgbn;
using namespace pgbn::infixExpr;

// FIXME: Err Process

#define CKERR() if (m_errCode != ErrCode::SUCCESS) return nullptr
#define BADTOK(type, msg) { std::cerr << __func__ << " " << __LINE__; badToken((type), (msg)); return nullptr; } PGZXB_PASS

#undef PGZXB_DEBUG_INFO_HEADER
#define PGZXB_DEBUG_INFO_HEADER __func__

// constructors
ExprParser::ExprParser() = default;

// public functions
ExprNode * ExprParser::parse() {
    return parseExpr();
}

// private functions
ExprNode * ExprParser::parseExpr() {
    return parseConditional();
}

ExprNode * ExprParser::parseConditional() {
    Node * condition = parseBitOr(); CKERR();
    if (!consumeToken(TokenType::QMARK)) return condition;
    
    Node * then = parseExpr(); CKERR();
    if (!consumeToken(TokenType::COLON)) BADTOK(TokenType::COLON, "expected ':'");
    Node * else_ = parseExpr(); CKERR();
    return m_pNodePool->newIfElseNode(condition, then, else_);
}

ExprNode * ExprParser::parseBitOr() {
    Node * node = parseBitXor();
    while (consumeToken(TokenType::OR)) {
        node = m_pNodePool->newBinOpNode(TokenType::OR, node, parseBitXor());
    }
    return node;
}

ExprNode * ExprParser::parseBitXor() {
    Node * node = parseBitAnd();
    while (consumeToken(TokenType::XOR)) {
        node = m_pNodePool->newBinOpNode(TokenType::XOR, node, parseBitAnd());
    }
    return node;
}

ExprNode * ExprParser::parseBitAnd() {
    Node * node = parseEqu();
    while (consumeToken(TokenType::AND)) {
        node = m_pNodePool->newBinOpNode(TokenType::AND, node, parseEqu());
    }
    return node;
}

ExprNode * ExprParser::parseEqu() {
    Node * node = parseCmp();
    while (true) {
        if (consumeToken(TokenType::EQU))
            node = m_pNodePool->newBinOpNode(TokenType::EQU, node, parseCmp());
        else if (consumeToken(TokenType::NEQU))
            node = m_pNodePool->newBinOpNode(TokenType::NEQU, node, parseCmp());
        return node;
    }
    return nullptr;
}

ExprNode * ExprParser::parseCmp() {
    Node * node = parseShift();
    while (true) {
        if (consumeToken(TokenType::LT))
            node = m_pNodePool->newBinOpNode(TokenType::LT, node, parseShift());
        else if (consumeToken(TokenType::LE))
            node = m_pNodePool->newBinOpNode(TokenType::LT, node, parseShift());
        else if (consumeToken(TokenType::GT))
            node = m_pNodePool->newBinOpNode(TokenType::LT, node, parseShift());
        else if (consumeToken(TokenType::GE))
            node = m_pNodePool->newBinOpNode(TokenType::LT, node, parseShift());
        else return node;
    }
    return nullptr;
}

ExprNode * ExprParser::parseShift() {
    Node * node = parseAddSub();
    while (true) {
        if (consumeToken(TokenType::SHIFTLEFT))
            node = m_pNodePool->newBinOpNode(TokenType::SHIFTLEFT, node, parseAddSub());
        else if (consumeToken(TokenType::SHIFTRIGHT))
            node = m_pNodePool->newBinOpNode(TokenType::SHIFTRIGHT, node, parseAddSub());
        else return node;
    }
    return nullptr;
}

ExprNode * ExprParser::parseAddSub() {
    Node * node = parseMulDivMod();
    while (true) {
        if (consumeToken(TokenType::ADD))
            node = m_pNodePool->newBinOpNode(TokenType::ADD, node, parseMulDivMod());
        else if (consumeToken(TokenType::SUB))
            node = m_pNodePool->newBinOpNode(TokenType::SUB, node, parseMulDivMod());
        else return node;
    }
    return nullptr;
}

ExprNode * ExprParser::parseMulDivMod() {
    Node * node = parsePow();
    while (true) {
        if (consumeToken(TokenType::MUL))
            node = m_pNodePool->newBinOpNode(TokenType::MUL, node, parsePow());
        else if (consumeToken(TokenType::DIV))
            node = m_pNodePool->newBinOpNode(TokenType::DIV, node, parsePow());
        else if (consumeToken(TokenType::MOD))
            node = m_pNodePool->newBinOpNode(TokenType::MOD, node, parsePow());
        else return node;
    }
    return nullptr;
}

ExprNode* ExprParser::parsePow() {
    Node* node = parseUnaryOp();
    while (true) {
        if (consumeToken(TokenType::POW))
            node = m_pNodePool->newBinOpNode(TokenType::POW, node, parseUnaryOp());
        else return node;
    }
    return nullptr;
}

ExprNode * ExprParser::parseUnaryOp() {
    if (consumeToken(TokenType::ADD))
        return m_pNodePool->newUnaryOpNode(TokenType::ADD, parseUnaryOp());
    else if (consumeToken(TokenType::SUB))
        return m_pNodePool->newUnaryOpNode(TokenType::SUB, parseUnaryOp());
    else if (consumeToken(TokenType::NOT))
        return m_pNodePool->newUnaryOpNode(TokenType::NOT, parseUnaryOp());
    return parsePostfixOp();
}

ExprNode * ExprParser::parsePostfixOp() {
    Node * node = parsePrim();
    while (true) {
        if (consumeToken(TokenType::FACT))
            node = m_pNodePool->newUnaryOpNode(TokenType::FACT, node);
        else return node;
    }
    return nullptr;
}

ExprNode * ExprParser::parsePrim() {
    if (consumeToken(TokenType::LEFTBRACKET)) {
        Node * node = parseExpr();
        if (!consumeToken(TokenType::RIGHTBRACKET))
            BADTOK(TokenType::RIGHTBRACKET, "expected ')'");
        return node;
    } else if (consumeToken(TokenType::LITERAL)) {
        return m_pNodePool->newLiteralNode(m_tokens[m_currTokenIndex - 1].val);
    } else if (consumeToken(TokenType::BUILTINCONSTANT)) {
        return m_pNodePool->newBuiltinValNode(m_tokens[m_currTokenIndex - 1].pSymbol);
    } else if (consumeToken(TokenType::BUILTINFUNC)) {
        auto & temp = m_tokens[m_currTokenIndex - 1];
        if (!consumeToken(TokenType::LEFTBRACKET)) BADTOK(TokenType::LEFTBRACKET, "Expected '()' To Call Function");
        return parseFuncCall(temp.pSymbol);
    }
    return nullptr;
}

ExprNode * ExprParser::parseFuncCall(SymbolTable::Symbol * symbol) {
    std::vector<Node*> argNodes;
    while (!consumeToken(TokenType::RIGHTBRACKET)) {
        if (argNodes.size() > 0 && !consumeToken(TokenType::COMMA)) BADTOK(TokenType::COMMA, "Expected ','");
        argNodes.push_back(parseExpr());
    }
    return m_pNodePool->newFunctionCallNode(symbol, std::move(argNodes));
}

bool ExprParser::consumeToken(TokenType type) {
    if (type != m_tokens[m_currTokenIndex].type) return false;
    
    ++m_currTokenIndex;
    return true;
}

Token & ExprParser::nextToken() {
    PGZXB_DEBUG_ASSERT(hasNextToken());
    return m_tokens[++m_currTokenIndex];
}

bool ExprParser::hasNextToken() const {
    return m_currTokenIndex + 1 < m_tokens.size();
}

void ExprParser::badToken(TokenType type, const StringArg & msg) const {
    m_errCode = ErrCode::PARSE_INFIXEXPR_BAD_TOKEN;
    PGBN_GetGlobalStatus() = ErrCode::PARSE_INFIXEXPR_BAD_TOKEN;
    std::any context = std::make_tuple(type, std::string(msg.data(), msg.size()));
    PGBN_GetGlobalStatus().setContext(context);
}
