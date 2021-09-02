#ifndef PGBIGNUMBER_INFIXEXPR_TOKEN_H
#define PGBIGNUMBER_INFIXEXPR_TOKEN_H

#include "fwd.h"
#include "../errInfos.h"
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
    NOT,             // !
    SHIFTRIGHT,      // >>
    SHIFTLEFT,       // <<
    BUILTINFUNC,     // <func-name>(<args-list>)
    BUILTINCONSTANT, // <constant-name>
    LITERAL,         // literal : BigInteger(Impl), Fraction(Impl) or Union{ BigInteger(Impl), Fraction(Impl) }
    LEFTBRACKET,     // '('
    RIGHTBRACKET,    // ')'
};

struct Token {
    struct Invalid {};

    TokenType type;
    Variant<Func, Value, Invalid> val{Invalid{}};
#ifdef PGBN_DEBUG
    std::string debugInfo;
#endif
};

namespace detail {
    template<typename STREAM>
    void skipWhiteChar(STREAM & stream) {
        // ' ' '\n' '\r' '\t'
        char ch = stream.peek();
        while (ch == 0 || ch == ' ' || ch == '\r' || ch == '\n' || ch == '\t') {
            stream.get();
            ch = stream.peek();
        }
    }
}

template<typename STREAM>
inline std::vector<Token> tokenizer(STREAM & stream) {
    std::vector<Token> res;

    while (!stream.eof()) {
        // 跳过空白符
        detail::skipWhiteChar(stream);

        // 解析
        char ch = stream.get();
        res.emplace_back();
        auto & currToken = res.back();
        auto & tokenType = currToken.type;
        auto & tokenVal = currToken.val;

#ifdef PGBN_DEBUG
        #define SET_TOKEN_DEBUG_INFO(info) currToken.debugInfo = (info);
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
        case '!' :
            tokenType = TokenType::FACT;
            SET_TOKEN_DEBUG_INFO("!");
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
        case '>' :
            if (stream.peek() == '>') {
                stream.get();
                tokenType = TokenType::SHIFTRIGHT;
                SET_TOKEN_DEBUG_INFO(">>");
            } else {
                GetGlobalStatus() = ErrCode::PARSE_INFIXEXPR_SHIFTOP_INVALID;
                return {};
            }
        case '<' :
            if (stream.peek() == '<') {
                stream.get();
                tokenType = TokenType::SHIFTRIGHT;
                SET_TOKEN_DEBUG_INFO("<<");
            } else {
                GetGlobalStatus() = ErrCode::PARSE_INFIXEXPR_SHIFTOP_INVALID;
                return {};
            }
        case '(' :
            tokenType = TokenType::LEFTBRACKET;
            SET_TOKEN_DEBUG_INFO("(");
            break;
        case ')' :
            tokenType = TokenType::RIGHTBRACKET;
            SET_TOKEN_DEBUG_INFO(")");
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
                            GetGlobalStatus() = ErrCode::PARSE_INFIXEXPR_NOTDEC_BUTREAL;
                            return {};
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

                tokenType = TokenType::LITERAL;
                SET_TOKEN_DEBUG_INFO("Literval(Base " + std::to_string(radix) + ") : " + literalBeforeDot + "." + literalAfterDot);
                // TODO:  解析数字设置Token的val
            } else if (std::isalpha(ch) || ch == '_') { // 内建常量或内建函数
                std::string sysmbolName;
                sysmbolName.push_back(ch);

                for (ch = stream.peek(); std::isalnum(ch) || ch == '_'; stream.get(), ch = stream.peek())
                    sysmbolName.push_back(ch);

                tokenType = TokenType::BUILTINCONSTANT;
                SET_TOKEN_DEBUG_INFO("Builtin Sysmbol : " + sysmbolName);
                // token = TokenType::BUILTINFUNC;
                // TODO: 去SysmbolTable中查找符号设置Token的val
            } else {
                GetGlobalStatus() = ErrCode::PARSE_INFIXEXPR_CHAR_INVALID;
                return {};
            }
        }
    }
    
    return res;
}

PGBN_INFIXEXPR_NAMESPACE_END
#endif // !PGBIGNUMBER_INFIXEXPR_TOKEN_H