#ifndef PGBIGNUMBER_FRACTION_H
#define PGBIGNUMBER_FRACTION_H

#include "fwd.h"
#include "Status.h"
#include "errInfos.h"
#include "BigInteger.h"
#include "algorithm.h"
#include <string>
#include <type_traits>
PGBN_NAMESPACE_START
namespace experimental {

class Fraction;
Fraction operator+(const Fraction &left, const Fraction &right);
Fraction operator-(const Fraction &left, const Fraction &right);
Fraction operator*(const Fraction &left, const Fraction &right);
Fraction operator/(const Fraction &left, const Fraction &right);

// Minimum API (for my homework)
class Fraction {
public:
    Fraction() = default;
    Fraction(const BigInteger &num, const BigInteger &den = BigInteger{1}, bool *ok = nullptr) : 
      m_num(num),
      m_den(den) {
        if (den == 0) {
            if (ok) *ok = false;
            PGBN_GetGlobalStatus() = ErrCode::DIVBY0;
            return;
        }
        if (ok) *ok = true;
        PGBN_GetGlobalStatus() = ErrCode::SUCCESS;
        fix();
    }

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
        PGBN_GetGlobalStatus() = ErrCode::SUCCESS;
        fix();
    }

    const BigInteger &getNum() const {
        return m_num;
    }

    const BigInteger &getDen() const {
        return m_den;
    }

    std::string toString(int radix = 10) const {
        int numSignum = m_num.cmp(0);
        int denSignum = m_den.cmp(0);
        int signum = numSignum * denSignum;
        return (signum == -1 ? "-" : "") +
               m_num.absoluted().toString(radix) + "/" +
               m_den.absoluted().toString(radix);
    }

    std::string toDecimal(int maxDecimalDigitCount = 8) const {
        auto q = m_num.absoluted() % m_den.absoluted();
        std::string digitalDigitsStr;
        if(maxDecimalDigitCount > 0) {
            auto mul = BigInteger("1" + std::string(maxDecimalDigitCount, '0'));
            digitalDigitsStr.append(".")
                .append((q * mul / m_den).toString());
            if (maxDecimalDigitCount > digitalDigitsStr.size()) {
                digitalDigitsStr.append(std::string(maxDecimalDigitCount - digitalDigitsStr.size(), '0'));
            }
        }
        return (m_num / m_den).toString(10) + digitalDigitsStr;
    }

    Fraction reciprocal(bool *ok = nullptr) const {
        if (m_den == 0) {
            if (ok) *ok = false;
            PGBN_GetGlobalStatus() = ErrCode::DIVBY0;
            return {};
        }
        Fraction result = *this;
        std::swap(result.m_num, result.m_den);
        if (ok) *ok = true;
        PGBN_GetGlobalStatus() = ErrCode::SUCCESS;
        return result;
    }

    friend Fraction operator+(const Fraction &left, const Fraction &right);
    friend Fraction operator-(const Fraction &left, const Fraction &right);
    friend Fraction operator*(const Fraction &left, const Fraction &right);
    friend Fraction operator/(const Fraction &left, const Fraction &right);
private:
    void fix() {
        PGZXB_DEBUG_ASSERT(m_den != 0);
        auto g = algo::gcd(m_num.absoluted(), m_den.absoluted());
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

Fraction operator-(const Fraction &left, const Fraction &right) {
    auto new_num = left.m_num * right.m_den - right.m_num * left.m_den;
    auto new_den = left.m_den * right.m_den;
    Fraction result;
    result.m_num = new_num;
    result.m_den = new_den;
    result.fix();
    return result;
}

Fraction operator*(const Fraction &left, const Fraction &right) {
    auto new_num = left.m_num * right.m_num;
    auto new_den = left.m_den * right.m_den;
    Fraction result;
    result.m_num = new_num;
    result.m_den = new_den;
    result.fix();
    return result;
}

Fraction operator/(const Fraction &left, const Fraction &right) {
    if (right.m_num == 0) {
        PGBN_GetGlobalStatus() = ErrCode::DIVBY0;
        return {};
    }
    auto new_num = left.m_num * right.m_den;
    auto new_den = left.m_den * right.m_num;
    Fraction result;
    result.m_num = new_num;
    result.m_den = new_den;
    result.fix();
    PGBN_GetGlobalStatus() = ErrCode::SUCCESS;
    return result;
}

} // namespace experimental
PGBN_NAMESPACE_END
#endif // !PGBIGNUMBER_FRACTION_H
