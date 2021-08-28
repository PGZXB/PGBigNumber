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
    constexpr Enum SUCCESS = 0;                       // 成功
    constexpr Enum DIVBY0 = 1;                        // 除零错误
    constexpr Enum RADIX_INVALID = 2;                 // 进制非法
    constexpr Enum NUMBER_STRING_EMPTY = 3;           // 表示数字的有效字符串为空
    constexpr Enum NUMBER_STRING_SIG_INVALID = 4;     // 表示数字的字符串符号表示非法
    constexpr Enum NUMBER_STRING_PARSE2NUM_ERROR = 5; // 字符串转数字错误
    constexpr Enum ARITHMETIC_OVERFLOW = 6;           // 计算过程中溢出
}

extern const ErrInfo errInfos[];
extern const SizeType errInfoCount;

PGBN_NAMESPACE_END
#endif // !PGBIGNUMBER_ERRINFOS_H