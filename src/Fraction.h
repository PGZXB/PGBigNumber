#ifndef PGBIGNUMBER_FRACTION_H
#define PGBIGNUMBER_FRACTION_H

#include "fwd.h"
#include "Status.h"
#include "errInfos.h"
#include "BigInteger.h"
#include "algorithm.h"
PGBN_NAMESPACE_START
namespace experimental {

class Fraction;
Fraction operator+(const Fraction &left, const Fraction &right);

// Minimum API (for my homework)
class Fraction {
public:
    void setNum(const BigInteger &num) {
        m_num = num;
        fix();
    }

    void setDen(const BigInteger &den, bool *ok = nullptr) {
        if (den == 0) {
            if (ok) *ok = false;
            PGBN_GetGlobalStatus() = ErrCode::DIVBY0;
            return;
        }
        m_den = den;
        if (ok) *ok = true;
        fix();
    }

    const BigInteger &getNum() const {
        return m_num;
    }

    const BigInteger &getDen() const {
        return m_den;
    }

    friend Fraction operator+(const Fraction &left, const Fraction &right);
private:
    void fix() {
        PGZXB_DEBUG_ASSERT(m_den != 0);
        auto g = algo::gcd(m_num, m_den);
        m_num /= g;
        m_den /= g;
    }
private:
    BigInteger m_num; // numerator
    BigInteger m_den{1}; // denominator
};

Fraction operator+(const Fraction &left, const Fraction &right) {
    auto new_num = left.m_num * right.m_den + right.m_num * left.m_den;
    auto new_den = left.m_den * right.m_den;
    Fraction result;
    result.m_num = new_num;
    result.m_den = new_den;
    result.fix();
    return result;
}

} // namespace experimental
PGBN_NAMESPACE_END
#endif // !PGBIGNUMBER_FRACTION_H
