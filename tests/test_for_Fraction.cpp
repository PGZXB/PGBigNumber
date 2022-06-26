#include "Fraction.h"

#define CHK(cond) PGZXB_DEBUG_ASSERT(cond)

using pgbn::BigInteger;
using namespace pgbn::experimental;

int main() {
    
    Fraction f1;
    f1.setNum(BigInteger{6});
    f1.setDen(BigInteger{4});
    CHK(f1.getNum() == 3);
    CHK(f1.getDen() == 2);

    Fraction f2;
    f2.setNum(BigInteger{4});
    f2.setDen(BigInteger{3});

    // +
    auto f3 = f1 + f2;
    CHK(f3.getNum() == 17);
    CHK(f3.getDen() == 6);
    CHK(f3.toDecimal(6) == "2.833333");

    // -
    auto f4 = f1 - f3;
    CHK(f4.getNum().absoluted() == 4);
    CHK(f4.getDen().absoluted() == 3);
    CHK(f4.getNum() * f4.getDen() < 0);
    CHK(f4.toString() == "-4/3");
    CHK(f4.toDecimal(3) == "-1.333");

    // *
    auto f5 = f1 * f2 * f3;
    CHK(f5.getNum() == 17);
    CHK(f5.getDen() == 3);
    
    // /
    auto f6 = f5 / (f2 * f3);
    CHK(f6.getNum() == 3);
    CHK(f6.getDen() == 2);

    return 0;    
}
