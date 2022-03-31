## PGBigNumber
Big number library written in C++17

### Feature
* +, -, *, /, mod, &, |, ~, ==, >=, >, ... (more operations)

* eval("\<a valid infix expression\>") (support calling builtin functions & ref builtin constants)

### Build
#### On Linux
```shell
git clone https://github.com/PGZXB/PGBigNumber.git PGBigNumber
cd PGBigNumber
mkdir build
cd build
cmake ..
make
./tests/test_for_calcu # Optional, to verify correctness 
cd ..
cd ..
```
#### On Windows
1. Clone the repo
2. Run cmake (generate VS project)
3. Open project & Build `ALL_BUILD`
### Usage

Set PGBigNumber/include as include dir.

Link static lib `PGBigNumber` (and `PGBigNumberBindingC` for C).
* simple.cpp
```C++
#include <cassert>
#include <iostream>

#include "PGBigNumber/BigInteger.h"
#include "PGBigNumber/expr.h"

using namespace pgbn;
using namespace pgbn::expr;

int main() {

    BigInteger a("1000000000000000000000000000000000000");
    BigInteger b("2");
    BigInteger c("-1000000000000000000000000000000000000");

    auto d = a * b;
    assert(d.toString() == "2000000000000000000000000000000000000");
    d /= c;
    assert(d.toString() == "-2");

    auto expr = pgfmt::format("{0} * {1} / {2} == {3}",
        a.toString(), b.toString(), c.toString(), d.toString());
    std::cout << expr << '\n';

    auto eval_expr = eval(expr);
    assert(eval_expr.isOne() && eval_expr.toString() == "1");
    std::cout << pgfmt::format("{0}: {1}", expr, eval_expr.as<bool>()) << '\n';

    return 0;
}
```
* simple.c
```C
#include <stdio.h>
#include <string.h>
#include <assert.h>

#include "PGBigNumber/pgbn.h"

int main() {

    pgbn_BigInteger_t *a = pgbn_BigInteger_CreateWithStr("1000000000000000000000000000000000000", 10);
    pgbn_BigInteger_t *b = pgbn_BigInteger_CreateWithStr("2", 10);
    pgbn_BigInteger_t *c = pgbn_BigInteger_CreateWithStr("-1000000000000000000000000000000000000", 10);

    pgbn_String_t *a_str = pgbn_BigInteger_ToString(a, 10);
    pgbn_String_t *b_str = pgbn_BigInteger_ToString(b, 10);
    pgbn_String_t *c_str = pgbn_BigInteger_ToString(c, 10);

    // d = a * b
    pgbn_BigInteger_t *d = pgbn_BigInteger_CreateWithCopying(a);
    pgbn_BigInteger_MulAssign(d, b);
    
    pgbn_String_t *d_str = pgbn_BigInteger_ToString(d, 10);
    assert(strcmp((pgbn_String_CStr(d_str)), "2000000000000000000000000000000000000") == 0);
    pgbn_String_Destroy(d_str);
    
    // d /= c
    pgbn_BigInteger_DivAssign(d, c);
    
    d_str = pgbn_BigInteger_ToString(d, 10);
    assert(strcmp((pgbn_String_CStr(d_str)), "-2") == 0);

    printf("%s * %s / %s == %s",
        pgbn_String_CStr(a_str),
        pgbn_String_CStr(b_str),
        pgbn_String_CStr(c_str),
        pgbn_String_CStr(d_str)
    );

    // Destroy
    pgbn_BigInteger_Destroy(a);
    pgbn_BigInteger_Destroy(b);
    pgbn_BigInteger_Destroy(c);
    pgbn_BigInteger_Destroy(d);

    pgbn_String_Destroy(a_str);
    pgbn_String_Destroy(b_str);
    pgbn_String_Destroy(c_str);
    pgbn_String_Destroy(d_str);
    return 0;
}
```

More examples in [examples/](https://github.com/PGZXB/PGBigNumber/tree/master/examples) (and [tests/](https://github.com/PGZXB/PGBigNumber/tree/master/tests))
