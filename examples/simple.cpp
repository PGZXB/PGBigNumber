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
