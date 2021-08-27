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
    constexpr Enum SUCCESS = 0;
    constexpr Enum DIVBY0 = 1;
}

extern const ErrInfo errInfos[];
extern const SizeType errInfoCount;

PGBN_NAMESPACE_END
#endif // !PGBIGNUMBER_ERRINFOS_H