#include "errInfos.h"

#undef PGZXB_DEBUG_INFO_HEADER
#define PGZXB_DEBUG_INFO_HEADER "[DEBUG] "

#include <cstdio>

// struct ErrInfo {
//     Enum code;
//     const char * info;
//     Callback callback;
// };

PGBN_NAMESPACE_START
namespace detail {

static inline void printErrInfo(pg::Enum code, const char * info) {
    std::fprintf(stderr, "[%" PRI_ENUM "]%s\n", code, info);
}

static void nullCallback(pg::Enum, const char *) {
}

static void processStatusExit(pg::Enum code, const char * info) {
    printErrInfo(code, info);
    exit(code);
}

static void debugPrint(pg::Enum code, const char * info) {
    PGZXB_DEBUG_Print(pgfmt::format("[{0}]{1}", code, info));
}

}

// err-infos
const ErrInfo errInfos[] = {
    { ErrCode::SUCCESS                      , "[PGBN-Status: OK]Success"                        , pgbn::detail::nullCallback      },
    { ErrCode::DIVBY0                       , "[PGBN-Status: Error]Divide By Zero"              , pgbn::detail::processStatusExit },
    { ErrCode::RADIX_INVALID                , "[PGBN-Status: Error]Radix Invalid"               , pgbn::detail::debugPrint        },
    { ErrCode::NUMBER_STRING_EMPTY          , "[PGBN-Status: Error]Number String Empty"         , pgbn::detail::debugPrint        },
    { ErrCode::NUMBER_STRING_SIG_INVALID    , "[PGBN-Status: Error]Number String Sig Invalid"   , pgbn::detail::debugPrint        },
    { ErrCode::NUMBER_STRING_PARSE2NUM_ERROR, "[PGBN-Status: Error]Prase String To Number Error", pgbn::detail::debugPrint        },
    { ErrCode::ARITHMETIC_OVERFLOW          , "[PGBN-Status: Error]Arithmetic Overflow"         , pgbn::detail::debugPrint        },
};
const SizeType errInfoCount = sizeof(errInfos) / sizeof(*errInfos);

PGBN_NAMESPACE_END
