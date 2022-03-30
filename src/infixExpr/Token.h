#ifndef PGBIGNUMBER_INFIXEXPR_TOKEN_H
#define PGBIGNUMBER_INFIXEXPR_TOKEN_H

#include "../errInfos.h"
#include "fwd.h"
#include "SymbolTable.h"
#include <cctype>
PGBN_INFIXEXPR_NAMESPACE_START

enum class TokenType : Enum {
    INVALID = 0,     // invalid token
    ADD,             // +
    SUB,             // -
    MUL,             // *
    DIV,             // /
    FACT,            // !
    POW,             // **
    AND,             // &
    OR,              // |
    XOR,             // ^
    NOT,             // ~
    SHIFTRIGHT,      // >>
    SHIFTLEFT,       // <<
    BUILTINFUNC,     // <func-name>(<args-list>)
    BUILTINCONSTANT, // <constant-name>
    LITERAL,         // literal : BigInteger(Impl), Fraction(Impl) or Union{ BigInteger(Impl), Fraction(Impl) }
    LEFTBRACKET,     // '('
    RIGHTBRACKET,    // ')'
    COMMA,           // ,
    MOD,             // %
    EQU,             // ==
    NEQU,            // !=
    LT,              // <
    LE,              // <=
    GT,              // >
    GE,              // >=
    QMARK,           // ?
    COLON,           // :
    TAIL
};

struct ExprNode;

struct Token {
    TokenType type;
    SymbolTable::Symbol * pSymbol = nullptr;
    Value val;
    ExprNode * nptr;
#ifdef PGBN_DEBUG
    std::string debugInfo;
#endif
};

// FIXME: Support SelfDefined-Hooks
template<typename STREAM>
inline std::vector<Token> tokenizer(STREAM & stream, const Hooks & hooks) { // FIXME: vector<Token> -> Lazy-Calcu(getNextToken)
    std::vector<Token> res;

    while (!stream.eof()) {
        // 跳过空白符
        utils::skipWhiteChar(stream);

        // 解析
        char ch = stream.get();
        res.emplace_back();
        auto & currToken = res.back();
        auto & tokenType = currToken.type;
        auto & tokenVal = currToken.val;
        auto & tokenPSymbol = currToken.pSymbol;

#ifdef PGBN_DEBUG
        #define SET_TOKEN_DEBUG_INFO(info) currToken.debugInfo = (info)
#else
        #define SET_TOKEN_DEBUG_INFO(info) PGZXB_PASS
#endif

        switch (ch) {
        case '+' :
            tokenType = TokenType::ADD;
            SET_TOKEN_DEBUG_INFO("+");
            break;
        case '-' :
            tokenType = TokenType::SUB;
            SET_TOKEN_DEBUG_INFO("-");
            break;
        case '*' :
            if (stream.peek() == '*') {
                stream.get();
                tokenType = TokenType::POW;
                SET_TOKEN_DEBUG_INFO("POW");
            } else {
                tokenType = TokenType::MUL;
                SET_TOKEN_DEBUG_INFO("*");
            }
            break;
        case '/' :
            tokenType = TokenType::DIV;
            SET_TOKEN_DEBUG_INFO("/");
            break;
        case '%' :
            tokenType = TokenType::MOD;
            SET_TOKEN_DEBUG_INFO("%");
            break;
        case '!' :
            if (stream.peek() == '=') {
                stream.get();
                tokenType = TokenType::NEQU;
                SET_TOKEN_DEBUG_INFO("!=");
            } else {
                tokenType = TokenType::FACT;
                SET_TOKEN_DEBUG_INFO("!");
            }
            break;
        case '&' :
            tokenType = TokenType::AND;
            SET_TOKEN_DEBUG_INFO("&");
            break;
        case '|' :
            tokenType = TokenType::OR;
            SET_TOKEN_DEBUG_INFO("|");
            break;
        case '^' :
            tokenType = TokenType::XOR;
            SET_TOKEN_DEBUG_INFO("^");
            break;
        case '~' :
            tokenType = TokenType::NOT;
            SET_TOKEN_DEBUG_INFO("~");
            break;
        case '>' :
            if (stream.peek() == '>') {
                stream.get();
                tokenType = TokenType::SHIFTRIGHT;
                SET_TOKEN_DEBUG_INFO(">>");
            } else if (stream.peek() == '=') {
                stream.get();
                tokenType = TokenType::GE;
                SET_TOKEN_DEBUG_INFO(">=");
            } else {
                tokenType = TokenType::GT;
                SET_TOKEN_DEBUG_INFO(">");
            }
            break;
        case '<' :
            if (stream.peek() == '<') {
                stream.get();
                tokenType = TokenType::SHIFTLEFT;
                SET_TOKEN_DEBUG_INFO("<<");
            } else if (stream.peek() == '=') {
                stream.get();
                tokenType = TokenType::LE;
                SET_TOKEN_DEBUG_INFO("<=");
            } else {
                tokenType = TokenType::LT;
                SET_TOKEN_DEBUG_INFO("<");
            }
            break;
        case '=' :
            if (stream.peek() == '=') {
                stream.get();
                tokenType = TokenType::EQU;
                SET_TOKEN_DEBUG_INFO("==");
            } else {
                PGBN_GetGlobalStatus() = ErrCode::PARSE_INFIXEXPR_EQU_INVALID;
                return res;
            }
            break;
        case '(' :
            tokenType = TokenType::LEFTBRACKET;
            SET_TOKEN_DEBUG_INFO("(");
            break;
        case ')' :
            tokenType = TokenType::RIGHTBRACKET;
            SET_TOKEN_DEBUG_INFO(")");
            break;
        case ',' :
            tokenType = TokenType::COMMA;
            SET_TOKEN_DEBUG_INFO(",");
            break;
        case '?' :
            tokenType = TokenType::QMARK;
            SET_TOKEN_DEBUG_INFO("?");
            break;
        case ':' :
            tokenType = TokenType::COLON;
            SET_TOKEN_DEBUG_INFO(":");
            break;
        default :
            if (std::isdigit(ch)) { // 字面量
                int radix = 10;
                std::string literalBeforeDot;
                std::string literalAfterDot;
                if (ch == '0') {
                    // 036x1314141421

                    bool onlyZero = false;
                    if (stream.peek() == 'x') {
                        stream.get();
                        radix = 16;
                    } else if (std::isdigit(stream.peek())) {
                        char hiDigit = stream.get(), loDigit = -1;
                        if (std::isdigit(stream.peek())) {
                            loDigit = stream.get();
                        }
                        if (stream.peek() != 'x') {
                            radix = 8;
                            literalBeforeDot.push_back(hiDigit);
                            if (loDigit != -1) literalBeforeDot.push_back(loDigit);
                        } else {
                            if (loDigit == -1) { 
                                loDigit = hiDigit;
                                hiDigit = '0';
                            }
                            radix = (hiDigit - '0') * 10 + (loDigit - '0');

                            stream.get();
                        }
                    } else {
                        literalBeforeDot = "0";
                        onlyZero = true;
                    }

                    if (!onlyZero) {
                        for (ch = std::tolower(stream.peek()); std::isdigit(ch) || (ch >= 'a' && ch <= 'z'); stream.get(), ch = stream.peek())
                            literalBeforeDot.push_back(ch);
                        if (ch == '.') {
                            PGBN_GetGlobalStatus() = ErrCode::PARSE_INFIXEXPR_NOTDEC_BUTREAL;
                            return res;
                        }
                    }
                } else {
                    literalBeforeDot.push_back(ch);
                    for (ch = std::tolower(stream.peek()); std::isdigit(ch) || (ch >= 'a' && ch <= 'z'); stream.get(), ch = stream.peek())
                        literalBeforeDot.push_back(ch);
                    if (ch == '.') {
                        stream.get();
                        for (ch = std::tolower(stream.get()); std::isdigit(ch) || (ch >= 'a' && ch <= 'z'); stream.get(), ch = stream.peek())
                            literalAfterDot.push_back(ch);
                    }

                }

                if (!(radix >= 2 && radix <= 36)) {
                    PGBN_GetGlobalStatus() = ErrCode::PARSE_INFIXEXPR_RADIX_INVALID;
                    return res;
                }

                tokenType = TokenType::LITERAL;
                tokenVal = Value{};
                if (!hooks.literal2Value || !hooks.literal2Value(tokenVal, radix, literalBeforeDot, literalAfterDot)) {
                    PGBN_GetGlobalStatus() = ErrCode::PARSE_INFIXEXPR_LITERAL2NUM_ERROR;
                    return res;
                }
                SET_TOKEN_DEBUG_INFO(pgfmt::format("Literval(Base {0}) : {1}.{2} (Value : {3})", radix, literalBeforeDot, literalAfterDot, tokenVal));
            } else if (std::isalpha(ch) || ch == '_') { // 内建常量或内建函数
                std::string symbolName;
                symbolName.push_back(ch);

                for (ch = stream.peek(); std::isalnum(ch) || ch == '_'; stream.get(), ch = stream.peek())
                    symbolName.push_back(ch);

                SymbolTable::Symbol * pSymbol = SymbolTable::getInstance()->get(symbolName);
                if (pSymbol == nullptr) { // UndefinedSymbol
                    PGBN_GetGlobalStatus() = ErrCode::PARSE_INFIXEXPR_SYMBOL_UNDEFINED;
                    return res;
                } else {
                    if (pSymbol->is<Func>()) tokenType = TokenType::BUILTINFUNC;
                    else if (pSymbol->is<Value>()) tokenType = TokenType::BUILTINCONSTANT;
                    else { // InvalidBuiltinSymbol
                        PGBN_GetGlobalStatus() = ErrCode::PARSE_INFIXEXPR_BUILTIN_SYMBOL_INVALID;
                        return res;
                    }
                    tokenPSymbol = pSymbol;
                }
                SET_TOKEN_DEBUG_INFO(pgfmt::format("Builtin Symbol({0}) : {1} (Symbol : {2})",
                    tokenType == TokenType::BUILTINFUNC ? std::string("Function", 8) : std::string("Constant", 8),
                    symbolName,
                    *tokenPSymbol
                ));
            } else {
                PGBN_GetGlobalStatus() = ErrCode::PARSE_INFIXEXPR_CHAR_INVALID;
                return res;
            }
            break;
        }
    }

    res.push_back(Token{});
    res.back().type = TokenType::INVALID;

#undef SET_TOKEN_DEBUG_INFO
    return res;
}

PGBN_INFIXEXPR_NAMESPACE_END

namespace pg::util::stringUtil::__IN_fmtUtil {
    template<>
    inline std::string transToString<pgbn::infixExpr::detail::Symbol>(const pgbn::infixExpr::detail::Symbol & ele, const std::string &) {
        using namespace pgbn::infixExpr;
        if (ele.is<Func>()) {
            return "@BuiltinFunction";
        } else if (ele.is<Value>()) {
            return transToString(ele.as<Value>(), "");
        }

        return "<default-string>";
    }
}

#endif // !PGBIGNUMBER_INFIXEXPR_TOKEN_H
