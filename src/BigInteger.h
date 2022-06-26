//
// Created by PGZXB on 2021/4/10.
//
#ifndef PGBIGNUMBER_BIGINTEGER_H
#define PGBIGNUMBER_BIGINTEGER_H

#include "fwd.h"
#include <memory>
#include <type_traits>

PGBN_NAMESPACE_START

class Status;
class BigIntegerImpl;
class BigInteger;

class BigInteger {
public:
    // constructors
    // default
    BigInteger() noexcept;

    // construct from BigIntegerImpl
    BigInteger(std::unique_ptr<BigIntegerImpl> &&impl);

    // copy, only copy ptr-to-impl, copy base-data on write
    BigInteger(const BigInteger & other);

    // move
    BigInteger(BigInteger && other) noexcept;

    // from POD-integer : only for int64_t
    BigInteger(std::int64_t i);

    // from Human-String, don't support 'E', like 0|[1-9|A-Z|a-z][0-9|A-Z|a-z]*
    BigInteger(const StringArg & str, int radix = 10, bool * ok = nullptr);

    // from infix-expression, like "2021 ^ 23333 + (23! * (2^3 - 3))",
    // support +, -, *, /(floor-div), !(fact), **(pow), (, ), &(and), |(or), ^(xor), ~(not),
    //         and some-builtin-functions
    /* BigInteger(const ExprTree & tree); */
    // BigInteger(const StringArg & infixExpr, InfixExprMode mode, Status * status = nullptr);  // 最后再实现

    // from binary, 2's complement, default little endian
    explicit BigInteger(SizeType len, const void * bin, bool little = true);

    // destructor
    ~BigInteger();

    // copy assignment, only copy pImpl, using COW
    BigInteger & operator= (const BigInteger & other);

    // move assignment
    BigInteger & operator= (BigInteger && other) noexcept;

    // assign from other types
    BigInteger & operator= (std::int64_t i);
    BigInteger & assign(std::int64_t i);
    BigInteger & assign(const StringArg & str, int radix = 10, bool * ok = nullptr);
    /* BigInteger(const ExprTree & tree); */
    // BigInteger & assign(const StringArg & infixExpr, InfixExprMode mode, Status * status = nullptr);
    BigInteger & fromBinary(const void * bin, SizeType len, bool little = true); // from little endian 2's complement

    // to POD-integer
    std::int64_t getInt64(bool * ok = nullptr) const;
    bool tryGetInt64(std::int64_t & result) const;

    std::uint64_t getUInt64(bool * ok = nullptr) const;
    bool tryGetUInt64(std::uint64_t & result) const;

    // 获取二进制补码块, 返回的是低位到高位从0开始数的第i块uint32对应的补码
    std::uint32_t get2sComplement(SizeType i) const;

    // to POD-integer, get lower bits & to human-string(base10)
    template <typename T>
    T as() const;

    // to string
    std::string toString(int radix = 10) const;

    // to binary
    SizeType copyMagDataTo(void * dest, SizeType maxlen) const; // 绝对值的二进制

    // is-
    bool isZero() const;
    bool isOne() const;
    bool isNegOne() const;    
    bool isPositive() const;
    bool isNegative() const;
    bool isEven() const;
    bool isOdd() const;

    // operators overload :
    //      BI op BI, BI op pod-int, pod-int op BI  (BI : BigInteger, pod-int : POD-integer)
    //      ++, --, ~, +=, -=, *=, /=, %=, &=, |=, ^=, <<=, >>=, bool, -(negative), +(positive) : member-functions
    //      +, -, *, /, %, &, |, ^, <<, >> : friend-functions, make from op-ass operators
    //      ==, !=, <, <=, >, >= : friend-functions

    // ++, --, ~, +=, -=, *=, /=, %=, &=, |=, ^=, bool, -(negative), +(positive) : member-functions
    BigInteger & operator++ ();
    BigInteger operator++ (int);

    BigInteger & operator-- ();
    BigInteger operator-- (int);

    BigInteger operator~ () const;

    BigInteger & operator+= (const BigInteger & other);
    BigInteger & operator+= (std::int64_t i64);

    BigInteger & operator-= (const BigInteger & other);
    BigInteger & operator-= (std::int64_t i64);

    BigInteger & operator*= (const BigInteger & other);
    BigInteger & operator*= (std::int64_t i64);

    BigInteger & operator/= (const BigInteger & other);
    BigInteger & operator/= (std::int64_t i64);

    BigInteger & operator%= (const BigInteger & other);
    BigInteger & operator%= (std::int64_t i64);

    BigInteger & operator&= (const BigInteger & other);
    BigInteger & operator&= (std::int64_t i64);

    BigInteger & operator|= (const BigInteger & other);
    BigInteger & operator|= (std::int64_t i64);

    BigInteger & operator^= (const BigInteger & other);
    BigInteger & operator^= (std::int64_t i64);

    /* BigInteger & operator<<= (const BigInteger & other); */
    BigInteger & operator<<= (std::uint64_t u64);

    /* BigInteger & operator>>= (const BigInteger & other); */
    BigInteger & operator>>= (std::uint64_t u64);

    BigInteger operator+ () const;
    BigInteger operator- () const;
    BigInteger negated() const; // <=> operator-()
    BigInteger absoluted() const;
    BigInteger & negate();
    BigInteger & abs();
    BigInteger & notSelf();
    BigInteger & notBits();

    // cmp
    int cmp(std::int64_t other) const;
    int cmp(const BigInteger & other) const;

    // FIXME: use && and &, to get better performance
private:
    std::unique_ptr<BigIntegerImpl> m_pImpl;
};


// +, -, *, /, %, &, |, ^ : friend-functions, make from op-ass operators
BigInteger operator+ (const BigInteger & left, const BigInteger & right);
BigInteger operator+ (const BigInteger & left, std::int64_t right);
BigInteger operator+ (std::int64_t left, const BigInteger & right);

BigInteger operator- (const BigInteger & left, const BigInteger & right);
BigInteger operator- (const BigInteger & left, std::int64_t right);
BigInteger operator- (std::int64_t left, const BigInteger & right);

BigInteger operator* (const BigInteger & left, const BigInteger & right);
BigInteger operator* (const BigInteger & left, std::int64_t right);
BigInteger operator* (std::int64_t left, const BigInteger & right);

BigInteger operator/ (const BigInteger & left, const BigInteger & right);
BigInteger operator/ (const BigInteger & left, std::int64_t right);
BigInteger operator/ (std::int64_t left, const BigInteger & right);

BigInteger operator% (const BigInteger & left, const BigInteger & right);
BigInteger operator% (const BigInteger & left, std::int64_t right);
BigInteger operator% (std::int64_t left, const BigInteger & right);

BigInteger operator& (const BigInteger & left, const BigInteger & right);
BigInteger operator& (const BigInteger & left, std::int64_t right);
BigInteger operator& (std::int64_t left, const BigInteger & right);

BigInteger operator| (const BigInteger & left, const BigInteger & right);
BigInteger operator| (const BigInteger & left, std::int64_t right);
BigInteger operator| (std::int64_t left, const BigInteger & right);

BigInteger operator^ (const BigInteger & left, const BigInteger & right);
BigInteger operator^ (const BigInteger & left, std::int64_t right);
BigInteger operator^ (std::int64_t left, const BigInteger & right);

BigInteger operator<< (const BigInteger & left, std::int64_t right);
BigInteger operator>> (const BigInteger & left, std::int64_t right);

// ==, !=, <, <=, >, >= : friend-functions, from cmp-algorithm
bool operator== (const BigInteger & left, const BigInteger & right);
bool operator== (const BigInteger & left, std::int64_t right);
bool operator== (std::int64_t left, const BigInteger & right);

bool operator!= (const BigInteger & left, const BigInteger & right);
bool operator!= (const BigInteger & left, std::int64_t right);
bool operator!= (std::int64_t left, const BigInteger & right);

bool operator< (const BigInteger & left, const BigInteger & right);
bool operator< (const BigInteger & left, std::int64_t right);
bool operator< (std::int64_t left, const BigInteger & right);

bool operator<= (const BigInteger & left, const BigInteger & right);
bool operator<= (const BigInteger & left, std::int64_t right);
bool operator<= (std::int64_t left, const BigInteger & right);

bool operator> (const BigInteger & left, const BigInteger & right);
bool operator> (const BigInteger & left, std::int64_t right);
bool operator> (std::int64_t left, const BigInteger & right);

bool operator>= (const BigInteger & left, const BigInteger & right);
bool operator>= (const BigInteger & left, std::int64_t right);
bool operator>= (std::int64_t left, const BigInteger & right);

// hash
// TODO: std::hash

PGBN_NAMESPACE_END
#endif //PGBIGNUMBER_BIGINTEGER_H
