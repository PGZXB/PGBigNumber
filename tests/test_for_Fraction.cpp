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

    auto f3 = f1 + f2;
    CHK(f3.getNum() == 17);
    CHK(f3.getDen() == 6);

    return 0;    
}
