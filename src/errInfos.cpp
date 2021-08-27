#include "errInfos.h"

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

}

// err-infos
const ErrInfo errInfos[] = {
    {ErrCode::SUCCESS, "[PGBN-Status: OK]Success", pgbn::detail::nullCallback},
    {ErrCode::DIVBY0, "[PGBN-Status: Error]Divide By Zero", pgbn::detail::processStatusExit},
};
const SizeType errInfoCount = sizeof(errInfos) / sizeof(*errInfos);

PGBN_NAMESPACE_END
