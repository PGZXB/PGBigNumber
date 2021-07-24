//
// Created by PGZXB on 2021/4/10.
//
#ifndef PGBIGNUMBER_BIGINTEGER_H
#define PGBIGNUMBER_BIGINTEGER_H

#include "fwd.h"
#include <memory>

PGBN_NAMESPACE_START

class Status;
class BigIntegerImpl;

class BigInteger {
public:
    // constructors
    // default
    BigInteger() noexcept;

    // copy, only copy ptr-to-impl, copy base-data on write
    BigInteger(const BigInteger & other);

    // move
    BigInteger(BigInteger && other) noexcept;

    // from POD-integer : only for int64_t
    BigInteger(std::int64_t i);

    // from Human-String, don't support 'E', like 0|[1-9|A-F|a-f][0-9|A-F|a-f]*
    explicit BigInteger(const StringArg & str, int radix = 10, Status * status = nullptr);

    // from infix-expression, like "2021 ^ 23333 + (23! * (2^3 - 3))",
    // support +, -, *, /(floor-div), !(fact), ^(pow), (, ), &(and), |(or), ^(xor), ~(not),
    //         and self-defined-operators
    /* BigInteger(const ExprTree & tree); */
    BigInteger(const StringArg & infixExpr, InfixExprMode mode, Status * status = nullptr);  // 最后再实现

    // from binary, default little endian
    BigInteger(void * bin, SizeType len, bool little = true);

    // destructor
    ~BigInteger();

    // copy assignment, only copy pImpl, using COW
    BigInteger & operator= (const BigInteger & other);

    // move assignment
    BigInteger & operator= (BigInteger && other) noexcept;

    // assign from other types
    BigInteger & operator= (std::int64_t i);
    BigInteger & assign(std::int64_t i);
    BigInteger & assign(const StringArg & str, Status * status = nullptr);
    /* BigInteger(const ExprTree & tree); */
    BigInteger & assign(const StringArg & infixExpr, InfixExprMode mode, Status * status = nullptr);
    BigInteger & assign(void * bin, SizeType len, bool little = true);

    // to POD-integer
    std::int64_t getInt64(Status * status = nullptr) const;
    bool tryGetInt64(std::int64_t & result) const;

    std::uint64_t getUInt64(Status * status = nullptr) const;
    bool tryGetUInt64(std::uint64_t & result) const;

    // to POD-integer, get lower bits & to human-string(base10)
    template <typename T>
    T as();

    // to string
    std::string toString(int radix = 10) const;

    // to ByteStream & from ByteStream
    const void * data() const;
    SizeType dataBytes() const;
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

    BigInteger & operator<<= (const BigInteger & other);
    BigInteger & operator<<= (std::int64_t i64);

    BigInteger & operator>>= (const BigInteger & other);
    BigInteger & operator>>= (std::int64_t i64);

    BigInteger operator+ () const;
    BigInteger operator- () const;

    // FIXME : use && and &, to get better performance

    operator bool() const;  // return false if this is invalid or 0 else true

    // +, -, *, /, %, &, |, ^ : friend-functions, make from op-ass operators
    friend BigInteger operator+ (const BigInteger & left, const BigInteger & right);
    friend BigInteger operator+ (const BigInteger & left, std::int64_t right);
    friend BigInteger operator+ (std::int64_t left, const BigInteger & right);

    friend BigInteger operator- (const BigInteger & left, const BigInteger & right);
    friend BigInteger operator- (const BigInteger & left, std::int64_t right);
    friend BigInteger operator- (std::int64_t left, const BigInteger & right);

    friend BigInteger operator* (const BigInteger & left, const BigInteger & right);
    friend BigInteger operator* (const BigInteger & left, std::int64_t right);
    friend BigInteger operator* (std::int64_t left, const BigInteger & right);

    friend BigInteger operator/ (const BigInteger & left, const BigInteger & right);
    friend BigInteger operator/ (const BigInteger & left, std::int64_t right);
    friend BigInteger operator/ (std::int64_t left, const BigInteger & right);

    friend BigInteger operator% (const BigInteger & left, const BigInteger & right);
    friend BigInteger operator% (const BigInteger & left, std::int64_t right);
    friend BigInteger operator% (std::int64_t left, const BigInteger & right);

    friend BigInteger operator& (const BigInteger & left, const BigInteger & right);
    friend BigInteger operator& (const BigInteger & left, std::int64_t right);
    friend BigInteger operator& (std::int64_t left, const BigInteger & right);

    friend BigInteger operator| (const BigInteger & left, const BigInteger & right);
    friend BigInteger operator| (const BigInteger & left, std::int64_t right);
    friend BigInteger operator| (std::int64_t left, const BigInteger & right);

    friend BigInteger operator^ (const BigInteger & left, const BigInteger & right);
    friend BigInteger operator^ (const BigInteger & left, std::int64_t right);
    friend BigInteger operator^ (std::int64_t left, const BigInteger & right);

    friend BigInteger operator<< (const BigInteger & left, const BigInteger & right);
    friend BigInteger operator<< (const BigInteger & left, std::int64_t right);
    friend BigInteger operator<< (std::int64_t left, const BigInteger & right);

    friend BigInteger operator>> (const BigInteger & left, const BigInteger & right);
    friend BigInteger operator>> (const BigInteger & left, std::int64_t right);
    friend BigInteger operator>> (std::int64_t left, const BigInteger & right);

    // ==, !=, <, <=, >, >= : friend-functions, from cmp-algorithm
    friend bool operator== (const BigInteger & left, const BigInteger & right);
    friend bool operator== (const BigInteger & left, std::int64_t right);
    friend bool operator== (std::int64_t left, const BigInteger & right);

    friend bool operator!= (const BigInteger & left, const BigInteger & right);
    friend bool operator!= (const BigInteger & left, std::int64_t right);
    friend bool operator!= (std::int64_t left, const BigInteger & right);

    friend bool operator< (const BigInteger & left, const BigInteger & right);
    friend bool operator< (const BigInteger & left, std::int64_t right);
    friend bool operator< (std::int64_t left, const BigInteger & right);

    friend bool operator<= (const BigInteger & left, const BigInteger & right);
    friend bool operator<= (const BigInteger & left, std::int64_t right);
    friend bool operator<= (std::int64_t left, const BigInteger & right);

    friend bool operator> (const BigInteger & left, const BigInteger & right);
    friend bool operator> (const BigInteger & left, std::int64_t right);
    friend bool operator> (std::int64_t left, const BigInteger & right);

    friend bool operator>= (const BigInteger & left, const BigInteger & right);
    friend bool operator>= (const BigInteger & left, std::int64_t right);
    friend bool operator>= (std::int64_t left, const BigInteger & right);

    // inverse-bits
    void inverse();

public:
    // static functions
    template<typename T>
    static BigInteger valueOf(); // integer & string -> BigInteger

private:
    std::unique_ptr<BigIntegerImpl> m_pImpl;
};

PGBN_NAMESPACE_END
#endif //PGBIGNUMBER_BIGINTEGER_H
