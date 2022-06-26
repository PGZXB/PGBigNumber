#include "algorithm.h"
#include "BigInteger.h"

PGBN_NAMESPACE_START
namespace algo {

BigInteger gcd(const BigInteger &x, const BigInteger &y) {
    BigInteger a = x;
    BigInteger b = y;

    if(a > b) std::swap(a, b);
    while(a > 0) {
        auto q = b % a;
        b = std::move(a);
        a = std::move(q);
    }
    return b;
}

} // namespace algo
PGBN_NAMESPACE_END
