#ifndef PGBIGNUMBER_BIGINTEGERIMPL_H
#define PGBIGNUMBER_BIGINTEGERIMPL_H

#include "../include/PGBigNumber/fwd.h"
#include "Slice.h"
PGBN_NAMESPACE_START

namespace BNFlag {

constexpr Enum INVALID = 0;
constexpr Enum ZERO = 1U;
constexpr Enum POSITIVE = 1U << 1U;
constexpr Enum NEGATIVE = 1U << 2U;

}

class BigIntegerImpl {
public:

private:
    Enum m_flags = BNFlag::INVALID;
    Slice<std::uint32_t> m_mag;
};

PGBN_NAMESPACE_END
#endif // !PGBIGNUMBER_BIGINTEGERIMPL_H