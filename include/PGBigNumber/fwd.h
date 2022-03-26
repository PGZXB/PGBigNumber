//
// Created by PGZXB on 2021/4/10.
//
#ifndef PGBIGNUMBER_FWD_H
#define PGBIGNUMBER_FWD_H

#include <cinttypes>
#include <limits>
#include <string>
#include <string_view>
#include <functional>

#if defined(PGBN_DEBUG) && !defined(PGZXB_DEBUG) 
#define PGZXB_DEBUG
#elif !defined(PGBN_DEBUG)
#undef PGZXB_DEBUG
#endif

#include "pgfwd.h"
#include "pgdebug.h"

/* PGBN <=> PGBigNumber
   Author : PGZXB(pgzxb@qq.com)
    _______     ______  ______   ____  _____  
   |_   __ \  .' ___  ||_   _ \ |_   \|_   _| 
     | |__) |/ .'   \_|  | |_) |  |   \ | |   
     |  ___/ | |   ____  |  __'.  | |\ \| |   
    _| |_    \ `.___]  |_| |__) |_| |_\   |_  
   |_____|    `._____.'|_______/|_____|\____|

   BigNumber Library Wittien In C++17.
*/
#define PGBN_NAMESPACE_START namespace pg { namespace base { namespace bigNumber {
#define PGBN_NAMESPACE_END } } }
#define PGBN_EXPR_NAMESPACE_START PGBN_NAMESPACE_START namespace expr {
#define PGBN_EXPR_NAMESPACE_END } PGBN_NAMESPACE_END

#define PGBN_MIN_CPP_VERSION 201703L

// HELPER-MACRO
#define PGBN_PASS (void(0))
/*
 * BigInteger has a std::shared_ptr<BigIntegerImpl>, using COW
 * COW : Check the ref count of ptr to impl, copy if count != 1
 */
PGBN_NAMESPACE_START

constexpr struct InfixExprMode { } infixExprMode;

using SizeType = std::uint64_t;
using Byte = std::uint8_t;

using BigNumberUnitInteger = std::uint32_t;
using BigNumber2UnitInteger = std::uint64_t;

using StringArg = std::string_view;

using Enum = std::uint32_t;
constexpr std::uint16_t MAX_PER_ENUM = sizeof(Enum) * 8;

using Callback = std::function<void()>;
extern Callback NULL_CALLBACK;

inline bool checkLittleEndian() {
    char test[] = {'\x01', '\x02', '\x03', '\x04'};

    return *(int*)test == 0x04030201;
}

inline constexpr bool isLittleEndian() {  // FIXME : 待跨平台
#ifdef _MSC_VER
#if '\x01\x02\x03\x04' == 0x01020304
    return false;
#elif '\x01\x02\x03\x04' == 0x04030201
    return true;
#else
#error "???"
#endif
#else
#if __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
    return false;
#elif __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
    return true;
#endif
#endif
}

namespace err {

}
PGBN_NAMESPACE_END

namespace pgbn = ::pg::base::bigNumber;

#endif //PGBIGNUMBER_FWD_H
