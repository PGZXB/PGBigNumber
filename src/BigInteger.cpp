//
// Created by 42025 on 2021/4/10.
//
#include "../include/PGBigNumber/BigInteger.h"
#include "../include/PGBigNumber/Status.h"
#include "BigIntegerImpl.h"
#include "errInfos.h"

// using namespace pgbn;

PGBN_NAMESPACE_START namespace detail {

static inline BigInteger getU64Max() {
    BigInteger res;

    res.assign(std::numeric_limits<std::int64_t>::max());
    res <<= 1;
    res |= 0x1;

    return res;
}

} PGBN_NAMESPACE_END

// default
pgbn::BigInteger::BigInteger() noexcept : m_pImpl(new BigIntegerImpl{}) { }

// construct from BigIntegerImpl
pgbn::BigInteger::BigInteger(std::unique_ptr<BigIntegerImpl>&& impl) : m_pImpl(std::move(impl)) {

}

// copy, only copy ptr-to-impl, copy base-data on write
pgbn::BigInteger::BigInteger(const BigInteger & other) : m_pImpl(new BigIntegerImpl{*other.m_pImpl}) { }

// move
pgbn::BigInteger::BigInteger(BigInteger && other) noexcept : m_pImpl(std::move(other.m_pImpl)) {
    other.m_pImpl = nullptr;
}

// from POD-integer : only for int64_t
pgbn::BigInteger::BigInteger(std::int64_t i) : m_pImpl(new BigIntegerImpl{i}) { }

// from Human-String, don't support 'E', like 0|[1-9|A-Z|a-z][0-9|A-Z|a-z]*
pgbn::BigInteger::BigInteger(const pgbn::StringArg & str, int radix, bool * ok)
    : m_pImpl(new BigIntegerImpl{str, radix, ok}) { }

// from binary, 2's complement, default little endian
pgbn::BigInteger::BigInteger(SizeType len, const void * bin, bool little)
    : m_pImpl(new BigIntegerImpl{bin, len, little}) { }

// destructor
pgbn::BigInteger::~BigInteger() = default;

// copy assignment, only copy pImpl, using COW
pgbn::BigInteger & pgbn::BigInteger::operator= (const BigInteger & other) {
    if (this == &other) return *this;

    m_pImpl.reset(new BigIntegerImpl{*other.m_pImpl});
    return *this;
}

// move assignment
pgbn::BigInteger & pgbn::BigInteger::operator= (BigInteger && other) noexcept {
    if (this == &other) return *this;

    m_pImpl = std::move(other.m_pImpl);
    other.m_pImpl = nullptr;

    return *this;
}

// assign from other types
pgbn::BigInteger & pgbn::BigInteger::operator= (std::int64_t i) {
    m_pImpl->assign(i);
    return *this;
}

pgbn::BigInteger & pgbn::BigInteger::assign(std::int64_t i) {
    m_pImpl->assign(i);
    return *this;
}

pgbn::BigInteger & pgbn::BigInteger::assign(const StringArg & str, int radix, bool * ok) {
    m_pImpl->fromString(str, radix, ok);
    return *this;
}

pgbn::BigInteger & pgbn::BigInteger::fromBinary(const void * bin, SizeType len, bool little) {
    m_pImpl->assign(bin, len, little);
    return *this;
}

// to POD-integer
std::int64_t pgbn::BigInteger::getInt64(bool * ok) const {
    if (
        *this > std::numeric_limits<std::int64_t>::max() ||
        *this < std::numeric_limits<std::int64_t>::min()
    ) {
        PGBN_GetGlobalStatus() = ErrCode::ARITHMETIC_OVERFLOW;
        ok && (*ok = false);
        return std::numeric_limits<std::int64_t>::max();
    }

    auto u64 = m_pImpl->toU64();
    PGBN_GetGlobalStatus() = ErrCode::SUCCESS;
    ok && (*ok = true);
    return static_cast<std::int64_t>(u64);
}

bool pgbn::BigInteger::tryGetInt64(std::int64_t & result) const {
    bool ok = true;
    result = getInt64(&ok);
    return ok;
}

std::uint64_t pgbn::BigInteger::getUInt64(bool * ok) const {
    static const BigInteger MAXU64 = detail::getU64Max();

    if (
        m_pImpl->flagsContains(BNFlag::NEGATIVE) ||
        *this > MAXU64
    ) {
        PGBN_GetGlobalStatus() = ErrCode::ARITHMETIC_OVERFLOW;
        ok && (*ok = false);
        return std::numeric_limits<std::uint64_t>::max();
    }

    auto u64 = m_pImpl->toU64();
    PGBN_GetGlobalStatus() = ErrCode::SUCCESS;
    ok && (*ok = true);
    return static_cast<std::int64_t>(u64);
}

bool pgbn::BigInteger::tryGetUInt64(std::uint64_t & result) const {
    bool ok = true;
    result = getUInt64(&ok);
    return ok;
}

std::uint32_t pgbn::BigInteger::get2sComplement(SizeType i) const {
    return m_pImpl->getU32(i);
}

template <>
std::string pgbn::BigInteger::as<std::string>() const {
    return this->toString();
}

#define DEFINE_AS_INTEGER(type) \
    template <> \
    type pgbn::BigInteger::as<type>() const { \
        return static_cast<type>(this->m_pImpl->toU64()); \
    }

template <>
bool pgbn::BigInteger::as<bool>() const {
    return !this->m_pImpl->flagsContains(BNFlag::ZERO);
}

DEFINE_AS_INTEGER( char               );
DEFINE_AS_INTEGER( unsigned char      );
DEFINE_AS_INTEGER( int                );
DEFINE_AS_INTEGER( unsigned int       );
DEFINE_AS_INTEGER( short              );
DEFINE_AS_INTEGER( unsigned short     );
DEFINE_AS_INTEGER( long               );
DEFINE_AS_INTEGER( unsigned long      );
DEFINE_AS_INTEGER( long long          );
DEFINE_AS_INTEGER( unsigned long long );

// to string
std::string pgbn::BigInteger::toString(int radix) const {
    return m_pImpl->toString(radix);
}

// to binary
pgbn::SizeType pgbn::BigInteger::copyMagDataTo(void * dest, SizeType maxlen) const {
    return m_pImpl->copyMagDataTo(dest, maxlen);
}

// is-
bool pgbn::BigInteger::isZero() const { return m_pImpl->flagsContains(BNFlag::ZERO); }
bool pgbn::BigInteger::isOne() const { return m_pImpl->isOne(); }
bool pgbn::BigInteger::isNegOne() const { return m_pImpl->isNegOne(); }
bool pgbn::BigInteger::isPositive() const { return m_pImpl->flagsContains(BNFlag::POSITIVE); }
bool pgbn::BigInteger::isNegative() const { return m_pImpl->flagsContains(BNFlag::NEGATIVE); }
bool pgbn::BigInteger::isEven() const { return m_pImpl->isEven(); }
bool pgbn::BigInteger::isOdd() const { return m_pImpl->isOdd(); }

// ++, --, ~, +=, -=, *=, /=, %=, &=, |=, ^=, bool, -(negative), +(positive) : member-functions
pgbn::BigInteger & pgbn::BigInteger::operator++ () {
    m_pImpl->inc();
    return *this;
}

pgbn::BigInteger pgbn::BigInteger::operator++ (int) {
    BigInteger temp = *this;
    ++*this;
    return temp;
}

pgbn::BigInteger & pgbn::BigInteger::operator-- () {
    m_pImpl->dec();
    return *this;
}

pgbn::BigInteger pgbn::BigInteger::operator-- (int) {
    BigInteger temp = *this;
    --*this;
    return temp;
}

pgbn::BigInteger pgbn::BigInteger::operator~ () const {
    BigInteger temp = *this;
    temp.m_pImpl->notSelf();
    return temp;
}

pgbn::BigInteger & pgbn::BigInteger::operator+= (const BigInteger & other) { m_pImpl->addAssign(*other.m_pImpl); return *this; }
pgbn::BigInteger & pgbn::BigInteger::operator+= (std::int64_t i64)         { m_pImpl->addAssign(i64); return *this;   }

pgbn::BigInteger & pgbn::BigInteger::operator-= (const BigInteger & other) { m_pImpl->subAssign(*other.m_pImpl); return *this; }
pgbn::BigInteger & pgbn::BigInteger::operator-= (std::int64_t i64)         { m_pImpl->subAssign(i64); return *this;   }

pgbn::BigInteger & pgbn::BigInteger::operator*= (const BigInteger & other) { m_pImpl->mulAssign(*other.m_pImpl); return *this; }
pgbn::BigInteger & pgbn::BigInteger::operator*= (std::int64_t i64)         { m_pImpl->mulAssign(i64); return *this;   }

pgbn::BigInteger & pgbn::BigInteger::operator/= (const BigInteger & other) { m_pImpl->divAssign(*other.m_pImpl); return *this; }
pgbn::BigInteger & pgbn::BigInteger::operator/= (std::int64_t i64)         { m_pImpl->divAssign(i64); return *this;   }

pgbn::BigInteger & pgbn::BigInteger::operator&= (const BigInteger & other) { m_pImpl->andAssign(*other.m_pImpl); return *this; }
pgbn::BigInteger & pgbn::BigInteger::operator&= (std::int64_t i64)         { m_pImpl->andAssign(i64); return *this;   }

pgbn::BigInteger & pgbn::BigInteger::operator|= (const BigInteger & other) { m_pImpl->orAssign(*other.m_pImpl); return *this;  }
pgbn::BigInteger & pgbn::BigInteger::operator|= (std::int64_t i64)         { m_pImpl->orAssign(i64); return *this;    }

pgbn::BigInteger & pgbn::BigInteger::operator^= (const BigInteger & other) { m_pImpl->xorAssign(*other.m_pImpl); return *this; }
pgbn::BigInteger & pgbn::BigInteger::operator^= (std::int64_t i64)         { m_pImpl->xorAssign(i64); return *this;   }

pgbn::BigInteger & pgbn::BigInteger::operator%= (const BigInteger & other) {
    BigIntegerImpl q;
    m_pImpl->modAssignAndQuotient(*other.m_pImpl, q);
    return *this;
}

pgbn::BigInteger & pgbn::BigInteger::operator%= (std::int64_t i64) {
    BigIntegerImpl q;
    m_pImpl->modAssignAndQuotient(i64, q);
    return *this;
}

pgbn::BigInteger & pgbn::BigInteger::operator<<= (std::uint64_t u64) { m_pImpl->shiftLeftAssign(u64); return *this;  }
pgbn::BigInteger & pgbn::BigInteger::operator>>= (std::uint64_t u64) { m_pImpl->shiftRightAssign(u64); return *this; }

pgbn::BigInteger pgbn::BigInteger::operator+ () const { return *this; }
pgbn::BigInteger pgbn::BigInteger::operator- () const { return BigInteger(*this).negate(); }

pgbn::BigInteger pgbn::BigInteger::negated() const { return -*this; }

pgbn::BigInteger & pgbn::BigInteger::negate() { m_pImpl->negate(); return *this; }
pgbn::BigInteger & pgbn::BigInteger::abs()    { m_pImpl->abs();    return *this; }

pgbn::BigInteger & pgbn::BigInteger::notSelf() {
    m_pImpl->notSelf();
    return *this;
}
pgbn::BigInteger & pgbn::BigInteger::notBits() {
    m_pImpl->notBits();
    return *this;
}

int pgbn::BigInteger::cmp(std::int64_t other) const       { return m_pImpl->cmp(BigIntegerImpl{other}); }
int pgbn::BigInteger::cmp(const BigInteger & other) const { return m_pImpl->cmp(*(other.m_pImpl));      }

// pgbn::BigInteger::operator bool() const { return !m_pImpl->flagsContains(BNFlag::ZERO); }

// +, -, *, /, %, &, |, ^ : friend-functions, make from op-ass operators
// FIXME: Can Be Better(mul and div)
pgbn::BigInteger pgbn::operator+ (const BigInteger & left, const BigInteger & right) { return BigInteger(left) += right; }
pgbn::BigInteger pgbn::operator+ (const BigInteger & left, std::int64_t right)       { return BigInteger(left) += right; }
pgbn::BigInteger pgbn::operator+ (std::int64_t left, const BigInteger & right)       { return BigInteger(left) += right; }

pgbn::BigInteger pgbn::operator- (const BigInteger & left, const BigInteger & right) { return BigInteger(left) -= right; }
pgbn::BigInteger pgbn::operator- (const BigInteger & left, std::int64_t right)       { return BigInteger(left) -= right; }
pgbn::BigInteger pgbn::operator- (std::int64_t left, const BigInteger & right)       { return BigInteger(left) -= right; }

pgbn::BigInteger pgbn::operator* (const BigInteger & left, const BigInteger & right) { return BigInteger(left) *= right; }
pgbn::BigInteger pgbn::operator* (const BigInteger & left, std::int64_t right)       { return BigInteger(left) *= right; }
pgbn::BigInteger pgbn::operator* (std::int64_t left, const BigInteger & right)       { return BigInteger(left) *= right; }

pgbn::BigInteger pgbn::operator/ (const BigInteger & left, const BigInteger & right) { return BigInteger(left) /= right; }
pgbn::BigInteger pgbn::operator/ (const BigInteger & left, std::int64_t right)       { return BigInteger(left) /= right; }
pgbn::BigInteger pgbn::operator/ (std::int64_t left, const BigInteger & right)       { return BigInteger(left) /= right; }

pgbn::BigInteger pgbn::operator% (const BigInteger & left, const BigInteger & right) { return BigInteger(left) %= right; }
pgbn::BigInteger pgbn::operator% (const BigInteger & left, std::int64_t right)       { return BigInteger(left) %= right; }
pgbn::BigInteger pgbn::operator% (std::int64_t left, const BigInteger & right)       { return BigInteger(left) %= right; }

pgbn::BigInteger pgbn::operator& (const BigInteger & left, const BigInteger & right) { return BigInteger(left) &= right; }
pgbn::BigInteger pgbn::operator& (const BigInteger & left, std::int64_t right)       { return BigInteger(left) &= right; }
pgbn::BigInteger pgbn::operator& (std::int64_t left, const BigInteger & right)       { return BigInteger(left) &= right; }

pgbn::BigInteger pgbn::operator| (const BigInteger & left, const BigInteger & right) { return BigInteger(left) |= right; }
pgbn::BigInteger pgbn::operator| (const BigInteger & left, std::int64_t right)       { return BigInteger(left) |= right; }
pgbn::BigInteger pgbn::operator| (std::int64_t left, const BigInteger & right)       { return BigInteger(left) |= right; }

pgbn::BigInteger pgbn::operator^ (const BigInteger & left, const BigInteger & right) { return BigInteger(left) ^= right; }
pgbn::BigInteger pgbn::operator^ (const BigInteger & left, std::int64_t right)       { return BigInteger(left) ^= right; }
pgbn::BigInteger pgbn::operator^ (std::int64_t left, const BigInteger & right)       { return BigInteger(left) ^= right; }

pgbn::BigInteger pgbn::operator<< (const BigInteger & left, std::int64_t right)      { return BigInteger(left) <<= right; }
pgbn::BigInteger pgbn::operator>> (const BigInteger & left, std::int64_t right)      { return BigInteger(left) >>= right; }

// ==, !=, <, <=, >, >= : friend-functions, from cmp-algorithm
bool pgbn::operator== (const BigInteger & left, const BigInteger & right) { return left.cmp(right) == 0; }
bool pgbn::operator== (const BigInteger & left, std::int64_t right)       { return left.cmp(right) == 0; }
bool pgbn::operator== (std::int64_t left, const BigInteger & right)       { return right.cmp(left) == 0; }

bool pgbn::operator!= (const BigInteger & left, const BigInteger & right) { return left.cmp(right) != 0; }
bool pgbn::operator!= (const BigInteger & left, std::int64_t right)       { return left.cmp(right) != 0; }
bool pgbn::operator!= (std::int64_t left, const BigInteger & right)       { return right.cmp(left) != 0; }

bool pgbn::operator<  (const BigInteger & left, const BigInteger & right) { return left.cmp(right) < 0;  }
bool pgbn::operator<  (const BigInteger & left, std::int64_t right)       { return left.cmp(right) < 0;  }
bool pgbn::operator<  (std::int64_t left, const BigInteger & right)       { return right.cmp(left) > 0;  }

bool pgbn::operator<= (const BigInteger & left, const BigInteger & right) { return left.cmp(right) <= 0; }
bool pgbn::operator<= (const BigInteger & left, std::int64_t right)       { return left.cmp(right) <= 0; }
bool pgbn::operator<= (std::int64_t left, const BigInteger & right)       { return right.cmp(left) >= 0; }

bool pgbn::operator>  (const BigInteger & left, const BigInteger & right) { return left.cmp(right) > 0;  }
bool pgbn::operator>  (const BigInteger & left, std::int64_t right)       { return left.cmp(right) > 0;  }
bool pgbn::operator>  (std::int64_t left, const BigInteger & right)       { return right.cmp(left) > 0;  }

bool pgbn::operator>= (const BigInteger & left, const BigInteger & right) { return left.cmp(right) >= 0; }
bool pgbn::operator>= (const BigInteger & left, std::int64_t right)       { return left.cmp(right) >= 0; }
bool pgbn::operator>= (std::int64_t left, const BigInteger & right)       { return right.cmp(left) <= 0; }
