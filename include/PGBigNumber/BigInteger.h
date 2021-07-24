//
// Created by PGZXB on 2021/4/10.
//
#ifndef PGBIGNUMBER_BIGINTEGER_H
#define PGBIGNUMBER_BIGINTEGER_H

#include <PGBigNumber/fwd.h>
#include <memory>


PGBN_NAMESPACE_START

class BigIntegerImpl;

class BigInteger {
//    static constexpr Enum InvalidFlag = 0U;
//    static constexpr Enum PositiveFlag = 1U << 1U;
//    static constexpr Enum NegativeFlag = 1U << 2U;
//    static constexpr Enum ZeroFlag = 1U << 3U;
//    static constexpr Enum OverFlowMIntFlag = 1U << 4U;
//    static constexpr Enum OverFlowMUIntFlag = 1U << 5U;
public:
    // constructors
    // default
    BigInteger() noexcept;

    // copy, only copy ptr-to-impl, copy base-data on write
    BigInteger(const BigInteger & other);

    // move
    BigInteger(BigInteger && other) noexcept;

    // from POD-integer : only for max_int_type and max_uint_type
    BigInteger(MaxInteger mi);
    BigInteger(MaxUInteger umi);

    // from Human-String, like "123" "+123" "-123" "1e3"
    explicit BigInteger(const StringArg & str, Enum * err = nullptr);

    // from infix-expression, like "2021 ^ 23333 + (23! * (2^3 - 3))",
    // support +, -, *, /(floor-div), !(fact), ^(pow), (, ), &(and), |(or), ^(xor), ~(not),
    //         and self-defined-operators
    /* BigInteger(const ExprTree & tree); */
    BigInteger(const StringArg & infixExpr, InfixExprMode mode, Enum * err = nullptr);  // 最后再实现, 结合另一个即将开始的项目(PGExpr)

    // from binary
    BigInteger(void * bin, SizeType len, bool little = true);

    // destructor
    ~BigInteger();

    // copy assignment, only copy pImpl, using COW
    BigInteger & operator= (const BigInteger & other);

    // move assignment
    BigInteger & operator= (BigInteger && other) noexcept;

    // assign from other types
    BigInteger & operator= (MaxInteger mi);
    BigInteger & operator= (MaxUInteger umi);
    BigInteger & assign(MaxInteger mi);
    BigInteger & assign(MaxUInteger umi);
    BigInteger & assign(const StringArg & str, Enum * err = nullptr);
    /* BigInteger(const ExprTree & tree); */
    BigInteger & assign(const StringArg & infixExpr, InfixExprMode mode, Enum * err = nullptr);
    BigInteger & assign(void * bin, SizeType len, bool little = true);

    // to POD-integer
    MaxInteger getInteger(Enum * err = nullptr) const;
    bool tryGetInteger(MaxInteger & result) const;

    MaxUInteger getUInteger(Enum * err = nullptr) const;
    bool tryGetUInteger(MaxUInteger & result) const;

    // get base-data
    const void * data() const;
    void * data();
    SizeType dataByte() const;
    void copyDataTo(void * dest, SizeType maxlen) const;

    // is-
    bool isValid() const;
    bool isZero() const;
    bool isPositive() const;
    bool isNegative() const;

    // operators overload :
    //      BI op BI, BI op pod-int, pod-int op BI  (BI : BigInteger, pod-int : POD-integer)
    //      ++, --, ~, +=, -=, *=, /=, %=, &=, |=, ^=, <<=, >>=, bool, -(negative), +(positive) : member-functions
    //      +, -, *, /, %, &, |, ^, <<, >> : friend-functions, make from op-ass operators
    //      ==, !=, <, <=, >, >= : friend-functions

    // ++, --, ~, +=, -=, *=, /=, %=, &=, |=, ^=, bool, -(negative), +(positive) : member-functions
    BigInteger & operator++ ();
    BigInteger operator++ (int);

    BigInteger & operator-- ();
    BigInteger & operator-- (int);

    BigInteger operator~ () const;

    BigInteger & operator+= (const BigInteger & other);
    BigInteger & operator+= (MaxInteger mi);
    BigInteger & operator+= (MaxUInteger umi);

    BigInteger & operator-= (const BigInteger & other);
    BigInteger & operator-= (MaxInteger mi);
    BigInteger & operator-= (MaxUInteger umi);

    BigInteger & operator*= (const BigInteger & other);
    BigInteger & operator*= (MaxInteger mi);
    BigInteger & operator*= (MaxUInteger umi);

    BigInteger & operator/= (const BigInteger & other);
    BigInteger & operator/= (MaxInteger mi);
    BigInteger & operator/= (MaxUInteger umi);

    BigInteger & operator%= (const BigInteger & other);
    BigInteger & operator%= (MaxInteger mi);
    BigInteger & operator%= (MaxUInteger umi);

    BigInteger & operator&= (const BigInteger & other);
    BigInteger & operator&= (MaxInteger mi);
    BigInteger & operator&= (MaxUInteger umi);

    BigInteger & operator|= (const BigInteger & other);
    BigInteger & operator|= (MaxInteger mi);
    BigInteger & operator|= (MaxUInteger umi);

    BigInteger & operator^= (const BigInteger & other);
    BigInteger & operator^= (MaxInteger mi);
    BigInteger & operator^= (MaxUInteger umi);

    BigInteger & operator<<= (const BigInteger & other);
    BigInteger & operator<<= (MaxInteger mi);
    BigInteger & operator<<= (MaxUInteger umi);

    BigInteger & operator>>= (const BigInteger & other);
    BigInteger & operator>>= (MaxInteger mi);
    BigInteger & operator>>= (MaxUInteger umi);

    BigInteger operator+ () const;
    BigInteger operator- () const;

    // FIXME : use && and &, to get better performance

    operator bool() const;  // return false if this is invalid or 0 else true

    // +, -, *, /, %, &, |, ^ : friend-functions, make from op-ass operators
    friend BigInteger operator+ (const BigInteger & left, const BigInteger & right);
    friend BigInteger operator+ (const BigInteger & left, MaxInteger right);
    friend BigInteger operator+ (const BigInteger & left, MaxUInteger right);
    friend BigInteger operator+ (MaxInteger left, const BigInteger & right);
    friend BigInteger operator+ (MaxUInteger left, const BigInteger & right);

    friend BigInteger operator- (const BigInteger & left, const BigInteger & right);
    friend BigInteger operator- (const BigInteger & left, MaxInteger right);
    friend BigInteger operator- (const BigInteger & left, MaxUInteger right);
    friend BigInteger operator- (MaxInteger left, const BigInteger & right);
    friend BigInteger operator- (MaxUInteger left, const BigInteger & right);

    friend BigInteger operator* (const BigInteger & left, const BigInteger & right);
    friend BigInteger operator* (const BigInteger & left, MaxInteger right);
    friend BigInteger operator* (const BigInteger & left, MaxUInteger right);
    friend BigInteger operator* (MaxInteger left, const BigInteger & right);
    friend BigInteger operator* (MaxUInteger left, const BigInteger & right);

    friend BigInteger operator/ (const BigInteger & left, const BigInteger & right);
    friend BigInteger operator/ (const BigInteger & left, MaxInteger right);
    friend BigInteger operator/ (const BigInteger & left, MaxUInteger right);
    friend BigInteger operator/ (MaxInteger left, const BigInteger & right);
    friend BigInteger operator/ (MaxUInteger left, const BigInteger & right);

    friend BigInteger operator% (const BigInteger & left, const BigInteger & right);
    friend BigInteger operator% (const BigInteger & left, MaxInteger right);
    friend BigInteger operator% (const BigInteger & left, MaxUInteger right);
    friend BigInteger operator% (MaxInteger left, const BigInteger & right);
    friend BigInteger operator% (MaxUInteger left, const BigInteger & right);

    friend BigInteger operator& (const BigInteger & left, const BigInteger & right);
    friend BigInteger operator& (const BigInteger & left, MaxInteger right);
    friend BigInteger operator& (const BigInteger & left, MaxUInteger right);
    friend BigInteger operator& (MaxInteger left, const BigInteger & right);
    friend BigInteger operator& (MaxUInteger left, const BigInteger & right);

    friend BigInteger operator| (const BigInteger & left, const BigInteger & right);
    friend BigInteger operator| (const BigInteger & left, MaxInteger right);
    friend BigInteger operator| (const BigInteger & left, MaxUInteger right);
    friend BigInteger operator| (MaxInteger left, const BigInteger & right);
    friend BigInteger operator| (MaxUInteger left, const BigInteger & right);

    friend BigInteger operator^ (const BigInteger & left, const BigInteger & right);
    friend BigInteger operator^ (const BigInteger & left, MaxInteger right);
    friend BigInteger operator^ (const BigInteger & left, MaxUInteger right);
    friend BigInteger operator^ (MaxInteger left, const BigInteger & right);
    friend BigInteger operator^ (MaxUInteger left, const BigInteger & right);

    friend BigInteger operator<< (const BigInteger & left, const BigInteger & right);
    friend BigInteger operator<< (const BigInteger & left, MaxInteger right);
    friend BigInteger operator<< (const BigInteger & left, MaxUInteger right);
    friend BigInteger operator<< (MaxInteger left, const BigInteger & right);
    friend BigInteger operator<< (MaxUInteger left, const BigInteger & right);

    friend BigInteger operator>> (const BigInteger & left, const BigInteger & right);
    friend BigInteger operator>> (const BigInteger & left, MaxInteger right);
    friend BigInteger operator>> (const BigInteger & left, MaxUInteger right);
    friend BigInteger operator>> (MaxInteger left, const BigInteger & right);
    friend BigInteger operator>> (MaxUInteger left, const BigInteger & right);

    // ==, !=, <, <=, >, >= : friend-functions
    friend bool operator== (const BigInteger & left, const BigInteger & right);
    friend bool operator== (const BigInteger & left, MaxInteger right);
    friend bool operator== (const BigInteger & left, MaxUInteger right);
    friend bool operator== (MaxInteger left, const BigInteger & right);
    friend bool operator== (MaxUInteger left, const BigInteger & right);

    friend bool operator!= (const BigInteger & left, const BigInteger & right);
    friend bool operator!= (const BigInteger & left, MaxInteger right);
    friend bool operator!= (const BigInteger & left, MaxUInteger right);
    friend bool operator!= (MaxInteger left, const BigInteger & right);
    friend bool operator!= (MaxUInteger left, const BigInteger & right);

    friend bool operator< (const BigInteger & left, const BigInteger & right);
    friend bool operator< (const BigInteger & left, MaxInteger right);
    friend bool operator< (const BigInteger & left, MaxUInteger right);
    friend bool operator< (MaxInteger left, const BigInteger & right);
    friend bool operator< (MaxUInteger left, const BigInteger & right);

    friend bool operator<= (const BigInteger & left, const BigInteger & right);
    friend bool operator<= (const BigInteger & left, MaxInteger right);
    friend bool operator<= (const BigInteger & left, MaxUInteger right);
    friend bool operator<= (MaxInteger left, const BigInteger & right);
    friend bool operator<= (MaxUInteger left, const BigInteger & right);

    friend bool operator> (const BigInteger & left, const BigInteger & right);
    friend bool operator> (const BigInteger & left, MaxInteger right);
    friend bool operator> (const BigInteger & left, MaxUInteger right);
    friend bool operator> (MaxInteger left, const BigInteger & right);
    friend bool operator> (MaxUInteger left, const BigInteger & right);

    friend bool operator>= (const BigInteger & left, const BigInteger & right);
    friend bool operator>= (const BigInteger & left, MaxInteger right);
    friend bool operator>= (const BigInteger & left, MaxUInteger right);
    friend bool operator>= (MaxInteger left, const BigInteger & right);
    friend bool operator>= (MaxUInteger left, const BigInteger & right);

    // inverse
    void inverse();

private:
    std::unique_ptr<BigIntegerImpl> m_pImpl;
};

PGBN_NAMESPACE_END
#endif //PGBIGNUMBER_BIGINTEGER_H
