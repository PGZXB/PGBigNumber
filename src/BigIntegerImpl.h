#ifndef PGBIGNUMBER_BIGINTEGERIMPL_H
#define PGBIGNUMBER_BIGINTEGERIMPL_H

#include "fwd.h"
#include "Slice.h"
PGBN_NAMESPACE_START

namespace BNFlag {

constexpr Enum INVALID = 0; // 非法
constexpr Enum ZERO = 1U; // 零标志
constexpr Enum POSITIVE = 1U << 1U; // 正数标志
constexpr Enum NEGATIVE = 1U << 2U; // 负数标志
// constexpr Enum DIRTY = 1U << 3U; // 写标志 [useless]
constexpr Enum FIRST_NZ_U32_INDEX_CALCUED = 1U << 4U; // 第一个非零U32是否被求值过标志

constexpr Enum LAZY_CALCU_FLAGS = FIRST_NZ_U32_INDEX_CALCUED;

}

#define BNFLAG

class Status;
namespace detail { class BigIntegerImplToStringHelper; }

class BigIntegerImpl {
    friend class detail::BigIntegerImplToStringHelper;
    static constexpr SizeType MAX_CHARS_PRE_DIGIT = 64;
public:
    // 构造
    BigIntegerImpl();

    BigIntegerImpl(const BigIntegerImpl & other); // 浅拷贝
    BigIntegerImpl(BigIntegerImpl && other) noexcept; // 浅拷贝

    // from int64
    BigIntegerImpl(std::int64_t i64);

    // from mag-array
    BigIntegerImpl(Slice<std::uint32_t> && slice, int signum); // signum, 1 : posi, -1 : neg, 0 : zero

    // from string
    BigIntegerImpl(const StringArg & str, int radix = 10, bool * ok = nullptr);

    // from binary, 2's complement, default little endian
    BigIntegerImpl(const void * bin, SizeType len, bool little = true);

    // 禁用赋值运算符
    BigIntegerImpl & operator= (const BigIntegerImpl & other);
    BigIntegerImpl & operator= (BigIntegerImpl && other) noexcept;

    // 赋值
    BigIntegerImpl & assign(const BigIntegerImpl & other); // Impl尽量提供简单的接口,
    BigIntegerImpl & assign(BigIntegerImpl && other);      // 尽量不以操作符的方式提供API
    BigIntegerImpl & assign(std::int64_t i);
    BigIntegerImpl & fromString(const StringArg & str, int radix = 10, bool * ok = nullptr);
    
    // /* BigIntegerImpl & assign(const ExprTree & tree); */
    // BigIntegerImpl & assign(const StringArg & infixExpr, InfixExprMode mode, Status * status = nullptr);
   
    // from binary, 2's complement, default little endian
    BigIntegerImpl & assign(const void * bin, SizeType len, bool little = true);

    // swap
    void swap(BigIntegerImpl & other) noexcept;

    // // 获取一组二进制补码形式的位, 要求传入的范围大小不超过64位 [useless]
    // std::uint64_t getBits(SizeType lo, SizeType hi); // [lo, hi)

    // // 获取绝对值的一组位, 要求传入的范围大小不超过64位 [useless]
    // std::uint64_t getMagBits(SizeType lo, SizeType hi); // [lo, hi)

    // 获取从低位到高位从零开始数的补码形式的第n个32位
    std::uint32_t getU32(SizeType n) const;

    // 获取字符串形式
    std::string toString(int radix) const;
    // std::string toTwosComplmentString() const; [useless]

    // // 以追加方式获取字符串, 要求RESULT有 : <不限制返回值> append(const char * str, SizeType len)
    // template<typename RESULT>
    // RESULT & toStringAppend(RESULT & res, int radix) const; [useless]

    // // 获取位数
    // SizeType minimalBitNumber() const; // 二进制补码形式 [useless]
    // SizeType minimalMagBitNumber() const; // 绝对值二进制补码 [useless]
    
    // 拷贝二进制位
    SizeType copyMagDataTo(void * dest, SizeType maxlen) const; // 绝对值二进制, little endian
    // /* SizeType copyDataTo(void * dest, SizeType maxlen) const; //补码形式, 上层调getU32即可, 不提供. */ [useless]

    // flags
    bool flagsContains(BNFLAG Enum flags) const;
    bool flagsEquals(BNFLAG Enum flags) const;

    // isZero
    bool isZero() const;

    // isOne
    bool isOne() const;

    // isNegOne
    bool isNegOne() const;

    // isOdd
    bool isOdd() const;

    // isEven
    bool isEven() const;

    // 自增自减
    void inc();
    void dec();

    // 加减乘除
    void addAssign(std::int64_t i64);
    void addAssign(const BigIntegerImpl & other);
    
    void subAssign(std::int64_t i64);
    void subAssign(const BigIntegerImpl & other);
    
    void mulAssign(std::int64_t i64);
    void mulAssign(const BigIntegerImpl & other);
    
    void divAssign(std::int64_t i64);
    void divAssign(const BigIntegerImpl & other);
    
    // 除法/取余相关操作
    void divideAssignAndReminder(const BigIntegerImpl & val, BigIntegerImpl & r);
    void divideAssignAndReminder(std::int64_t i64, BigIntegerImpl & r);

    void modAssignAndQuotient(const BigIntegerImpl & val, BigIntegerImpl & q);
    void modAssignAndQuotient(std::int64_t i64, BigIntegerImpl & q);

    // 位运算, 按补码形式进行运算
    void andAssign(std::int64_t i64);
    void andAssign(const BigIntegerImpl & other);

    void orAssign(std::int64_t i64);
    void orAssign(const BigIntegerImpl & other);

    void xorAssign(std::int64_t i64);
    void xorAssign(const BigIntegerImpl & other);

    void notSelf();
    void notBits();

    // 移位, 按照C++的标准, 正数逻辑移位, 负数算术移位(其实就都是算术移位)
    void shiftLeftAssign(std::uint64_t u64);
    void shiftRightAssign(std::uint64_t u64);

    // negate
    void negate();

    // abs
    void abs();

    // compare
    int cmp(const BigIntegerImpl & other) const; // less : -1, equals : 0, more : +1

    // get u32-count
    SizeType getMagU32Count() const;

    // get bit-count
    std::tuple<SizeType, SizeType> getMagBitCount() const; // return {r, q}, the result is r + q * 32

    // 转化为uint64_t, 二进制补码形式, 截断
    std::uint64_t toU64() const;

    // // inverse-bits inplace
    // void inverse(); [useless]
public:
    static void mul(BigIntegerImpl & res, const BigIntegerImpl & a, const BigIntegerImpl & b);
    static void div(BigIntegerImpl & q, BigIntegerImpl & r, const BigIntegerImpl & a, const BigIntegerImpl & b);
    static void div(BigIntegerImpl & q, BigIntegerImpl & r, const BigIntegerImpl & a, std::int64_t i64);
    // static BigIntegerImpl mulToomCook3(const BigIntegerImpl & a, const BigIntegerImpl & b); [useless]

private:
    static void mulKaratsuba(BigIntegerImpl & res, const BigIntegerImpl & a, const BigIntegerImpl & b);
    static void knuthDivImpl(BigIntegerImpl & q, BigIntegerImpl & r, const BigIntegerImpl & a, const BigIntegerImpl & b); // requires a > b

    void divideAssignAndReminderByU32(std::uint32_t u32, BigIntegerImpl & r);
    void modAssignAndQuotientByU32(std::uint32_t u32, BigIntegerImpl & r);

    void beginWrite(); // 初始化写操作

    SizeType getFirstNonZeroU32Index() const;

    void setFlagsToPositive();
    void setFlagsToNegative();
    void setFlagsToZero();
    bool hasSameSigFlag(const BigIntegerImpl & other) const;

    void beZero();
    void beOne();
    void beNegOne();

    std::vector<BigIntegerImpl> split(SizeType n, SizeType size) const;
private:
    mutable Enum m_flags = BNFlag::ZERO; // flags : 约定: assign(除了copy)后flags只有符号标志位, 其他写操作或懒求值操作均是修改flags的位
    Slice<std::uint32_t> m_mag{}; // 绝对值二进制, 去除前导零(高位 | 大下标)
    mutable SizeType m_firstNotZeroU32IndexLazy = 0; // 从低位开始数的第一个不为0的U32的下标
};

namespace detail {

// 因为next出来的字符串是倒序的(除法取余, 先取到低位), 其实这个玩意没啥用, 唉, 算了, 留着吧
class BigIntegerImplToStringHelper { // to help BigIntegerImpl::toString, 不帮助0
public:
    BigIntegerImplToStringHelper(const BigIntegerImpl & val, int radix)
    : m_temp(val), m_radix(radix) {
        constexpr int MIN_RADIX = 2; PGZXB_UNUSED(MIN_RADIX);
        constexpr int MAX_RADIX = 36; PGZXB_UNUSED(MAX_RADIX);
        PGZXB_DEBUG_ASSERT_EX("radix must in [2, 3,... ,36]", radix >= MIN_RADIX && radix <= MAX_RADIX);
    }

    bool hasNext() const { return !m_temp.flagsContains(BNFlag::ZERO) || m_index == 0; }
    std::tuple<SizeType, const char *, SizeType> next(); // {index, str, len}, no leading zeros
private:
    SizeType m_index = 0;
    BigIntegerImpl m_temp;
    char m_buf[BigIntegerImpl::MAX_CHARS_PRE_DIGIT + 1] = { 0 };
    const int m_radix;
};

}

inline void swap(BigIntegerImpl & a, BigIntegerImpl & b) noexcept {
    a.swap(b);
}

PGBN_NAMESPACE_END

namespace pg::util::stringUtil::__IN_fmtUtil {
    template<>
    inline std::string transToString<pgbn::BigIntegerImpl>(const pgbn::BigIntegerImpl & ele, const std::string & limit) {
        auto radix = std::atoi(limit.c_str());
        if (radix < 2 || radix > 36) radix = 10;
        return ele.toString(radix);
    }
}

#endif // !PGBIGNUMBER_BIGINTEGERIMPL_H
