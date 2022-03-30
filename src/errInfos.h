#ifndef PGBIGNUMBER_ERRINFOS_H
#define PGBIGNUMBER_ERRINFOS_H

#include "../include/PGBigNumber/fwd.h"
PGBN_NAMESPACE_START

struct ErrInfo {
    Enum code;
    const char * info;
    std::function<void(Enum, const char *)> callback;
};

namespace ErrCode {
    constexpr Enum SUCCESS = 0;                                     // 成功
    constexpr Enum DIVBY0 = 1;                                      // 除零错误
    constexpr Enum RADIX_INVALID = 2;                               // 进制非法
    constexpr Enum NUMBER_STRING_EMPTY = 3;                         // 表示数字的有效字符串为空
    constexpr Enum NUMBER_STRING_SIG_INVALID = 4;                   // 表示数字的字符串符号表示非法
    constexpr Enum NUMBER_STRING_PARSE2NUM_ERROR = 5;               // 字符串转数字错误
    constexpr Enum ARITHMETIC_OVERFLOW = 6;                         // 计算过程中溢出
    constexpr Enum PARSE_INFIXEXPR_SHIFTOP_INVALID = 7;             // 解析中缀表达式移位符号不合法
    constexpr Enum PARSE_INFIXEXPR_CHAR_INVALID = 8;                // 解析中缀表达式非法字符
    constexpr Enum PARSE_INFIXEXPR_NOTDEC_BUTREAL = 9;              // 解析中缀表达式非十进制确是实数(只允许整数)
    constexpr Enum PARSE_INFIXEXPR_SYMBOL_UNDEFINED = 10;           // 解析中缀表达式未定义的符号
    constexpr Enum PARSE_INFIXEXPR_BUILTIN_SYMBOL_INVALID = 11;     // 解析中缀表达式非法的内建符号
    constexpr Enum PARSE_INFIXEXPR_RADIX_INVALID = 12;              // 解析中缀表达式非法的进制
    constexpr Enum PARSE_INFIXEXPR_LITERAL2NUM_ERROR = 13;          // 解析中缀表达式非法的进制
    constexpr Enum PARSE_INFIXEXPR_EQU_INVALID = 14;                // 解析中缀表达式等于号非法
    constexpr Enum PARSE_INFIXEXPR_BAD_TOKEN = 15;                  // 解析中缀表达式非法的Token
    constexpr Enum PARSE_INFIXEXPR_BUILTINFUNC_CALL_NOT_MATCH = 16; // 解析中缀表达式调用内建不匹配
}

extern const ErrInfo errInfos[];
extern const SizeType errInfoCount;

PGBN_NAMESPACE_END
#endif // !PGBIGNUMBER_ERRINFOS_H
