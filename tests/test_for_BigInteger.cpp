#undef PGZXB_DEBUG
#define PGZXB_DEBUG

#include <iostream>
#include "BigInteger.h"

using pgbn::BigInteger;

std::ostream & operator<< (std::ostream & os, const BigInteger & val) {
    return os << val.toString();
}

#undef PGZXB_DEBUG_INFO_HEADER
#define PGZXB_DEBUG_INFO_HEADER "[TEST] "

int main () {

    BigInteger a("100000000000000000000000");
    BigInteger b("200000000000000000000000");
    BigInteger c("191919191919191911919191919999999999999112121");
    BigInteger d("1000");

    PGZXB_DEBUG_PrintVar(a);
    PGZXB_DEBUG_PrintVar(b);
    PGZXB_DEBUG_PrintVar(a + b);
    PGZXB_DEBUG_PrintVar(a - b);
    PGZXB_DEBUG_PrintVar(a * b);
    PGZXB_DEBUG_PrintVar(b / a);
    PGZXB_DEBUG_PrintVar(a / b);
    PGZXB_DEBUG_PrintVar(a > b);
    PGZXB_DEBUG_PrintVar(a <= b);
    PGZXB_DEBUG_PrintVar(a == 10);
    PGZXB_DEBUG_PrintVar(d == 1000);

    PGZXB_DEBUG_PrintVar(a * b + c * d - a + b / a);

    a = BigInteger("01010101010101010101010101", 2);
    b = BigInteger(a).notBits();

    PGZXB_DEBUG_PrintVar(a.toString(2));
    PGZXB_DEBUG_PrintVar(b.toString(2));
    PGZXB_DEBUG_PrintVar((a | b).toString(2));

    std::vector<char> v;
    while (!c.isZero()) {
        v.push_back('0' + (c % 10).getUInt64());
        c /= 10;
    }

    std::cout << "[TEST] c : ";
    for (auto iter = v.rbegin(); iter != v.rend(); ++iter) {
        std::cout << *iter;
    }
    std::cout << "\n";

    c.assign("191919191919191911919191919999999999999112121");

    PGZXB_DEBUG_PrintVar(c.as<bool>());
    PGZXB_DEBUG_PrintVar(c.as<int>());
    PGZXB_DEBUG_PrintVar(c.as<std::string>());
    PGZXB_DEBUG_PrintVar(d.as<long long>());
    PGZXB_DEBUG_PrintVar(d.as<unsigned>());
    PGZXB_DEBUG_PrintVar(d.as<bool>());
    PGZXB_DEBUG_PrintVar(d.as<std::string>());
    PGZXB_DEBUG_PrintVar((-d).as<std::int32_t>());

    return 0;
}
