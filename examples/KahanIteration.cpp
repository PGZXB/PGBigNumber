#include "PGBigNumber/Fraction.h"

using pgbn::experimental::Fraction;

double wrongKahanIteration(std::size_t n) {
    // y(1) = 4
    // y(2) = 4.25 = 17/4
    // y(n) = 108 - 815 / y(n-1) + 1500 / (y(n-1) * y(n-2))

    if (n == 1) return 4;
    if (n == 2) return 4.25;

    const auto d108 = 108.f;
    const auto d815 = 815.f;
    const auto d1500 = 1500.f;

    auto yn_2 = 4.f;   // y(n-2)
    auto yn_1 = 4.25f; // y(n-1)
    double yn{-1.f};       // y(n)
    for (std::size_t i = 3; i <= n; ++i) {
        yn = d108 - d815 / yn_1 + d1500 / (yn_1 * yn_2);
        yn_2 = yn_1;
        yn_1 = yn;
    }
    return yn;
}

Fraction kahanIteration(std::size_t n) {
    // y(1) = 4
    // y(2) = 4.25 = 17/4
    // y(n) = 108 - 815 / y(n-1) + 1500 / (y(n-1) * y(n-2))

    if (n == 1) return Fraction{4};
    if (n == 2) return Fraction{17, 4};

    const auto frac108 = Fraction{108};
    const auto frac815 = Fraction{815};
    const auto frac1500 = Fraction{1500};

    auto yn_2 = Fraction{4};     // y(n-2)
    auto yn_1 = Fraction{17, 4}; // y(n-1)
    Fraction yn;                 // y(n)
    for (std::size_t i = 3; i <= n; ++i) {
        yn = frac108 - frac815 / yn_1 + frac1500 / (yn_1 * yn_2);
        yn_2 = yn_1;
        yn_1 = yn;
    }
    return yn;
}

int main() {

    auto wrongY35 = wrongKahanIteration(35);
    std::cout << pgfmt::format("(wrong  , using `double`  ) y35 = {0:.10}\n", wrongY35);

    auto y35 = kahanIteration(35);
    std::cout << pgfmt::format("(correct, using `Fraction`) y35 = {0} = {1}\n", y35.toString(), y35.toDecimal(10));

    return 0;
}
