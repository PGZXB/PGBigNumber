#ifndef PGBIGNUMBER_BIGINTEGERIMPL_H
#define PGBIGNUMBER_BIGINTEGERIMPL_H

#include "../include/PGBigNumber/fwd.h"
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

class BigIntegerImpl {
public:
    // 构造
    BigIntegerImpl();

    explicit BigIntegerImpl(const BigIntegerImpl & other); // 浅拷贝
    explicit BigIntegerImpl(BigIntegerImpl && other) noexcept; // 浅拷贝

    BigIntegerImpl(std::int64_t i64);

    //// from binary, 2's complement, default little endian
    BigIntegerImpl(const void * bin, SizeType len, bool little = true);

    // 禁用赋值运算符
    BigIntegerImpl & operator= (const BigIntegerImpl & other) = delete; // Impl尽量提供简单的接口,
    BigIntegerImpl & operator= (BigIntegerImpl && other) = delete;      // 尽量不以操作符的方式提供API

    // 赋值
    BigIntegerImpl & assign(const BigIntegerImpl & other); // Impl尽量提供简单的接口,
    BigIntegerImpl & assign(BigIntegerImpl && other);      // 尽量不以操作符的方式提供API
    BigIntegerImpl & assign(std::int64_t i);
    // BigIntegerImpl & assign(const StringArg & str, Status * status = nullptr);
    // /* BigIntegerImpl & assign(const ExprTree & tree); */
    // BigIntegerImpl & assign(const StringArg & infixExpr, InfixExprMode mode, Status * status = nullptr);
   
    //// from binary, 2's complement, default little endian
    BigIntegerImpl & assign(const void * bin, SizeType len, bool little = true);

    // // 获取一组二进制补码形式的位, 要求传入的范围大小不超过64位 [useless]
    // std::uint64_t getBits(SizeType lo, SizeType hi); // [lo, hi)

    // // 获取绝对值的一组位, 要求传入的范围大小不超过64位 [useless]
    // std::uint64_t getMagBits(SizeType lo, SizeType hi); // [lo, hi)

    // 获取从低位到高位从零开始数的补码形式的第n个32位
    std::uint32_t getU32(SizeType n) const;

    // // 获取真值形式的字符串形式
    // std::string toString(int radix) const;

    // // 获取位数
    // SizeType minimalBitNumber() const; // 二进制补码形式
    // SizeType minimalMagBitNumber() const; // 绝对值二进制补码
    
    // // 拷贝mag的二进制位
    // SizeType copyMagDataTo(void * dest, SizeType maxlen) const; // 绝对值二进制
    // /* SizeType copyDataTo(void * dest, SizeType maxlen) const; //补码形式, 上层调getU32即可, 不提供. */

    // flags
    bool flagsContains(BNFLAG Enum flags) const;
    bool flagsEquals(BNFLAG Enum flags) const;

    // // 运算, 均为本地算法
    // void inc();
    // void dec();

    // void addAssign(std::int64_t i64);
    // void addAssign(const BigIntegerImpl & other);
    
    // void subAssign(std::int64_t i64);
    // void subAssign(const BigIntegerImpl & other);
    
    // void mulAssign(std::int64_t i64);
    // void mulAssign(const BigIntegerImpl & other);
    
    // void divAssign(std::int64_t i64);
    // void divAssign(const BigIntegerImpl & other);
    
    // std::tuple<std::unique_ptr<BigIntegerImpl>, std::unique_ptr<BigIntegerImpl>>
    //     divideAndReminder(const BigIntegerImpl & val); // 返回商和余数

    // 位运算, 按补码形式进行运算
    void andAssign(std::int64_t i64);
    void andAssign(const BigIntegerImpl & other);

    void orAssign(std::int64_t i64);
    void orAssign(const BigIntegerImpl & other);

    void xorAssign(std::int64_t i64);
    void xorAssign(const BigIntegerImpl & other);

    void notSelf();

    // // 移位, 按照C++的标准, 正数逻辑移位, 负数算术移位
    // void shiftLeft(std::uint64_t u64);
    // void shiftRight(std::uint64_t u64);

    // // compare
    // int cmp(const BigIntegerImpl & other); // less : -1, equals : 0, more : +1

    // // inverse-bits inplace
    // void inverse();

private:
    // BigIntegerImpl(const Slice<std::uint32_t> & slice, int signum); // signum, 1 : posi, -1 : neg, 0 : zero

    void beginWrite(); // 初始化写操作

    SizeType getFirstNonZeroU32Index() const;

    void setFlagsToPositive();
    void setFlagsToNegtaive();
    void setFlagsToZero();
private:
    mutable Enum m_flags = BNFlag::INVALID; // flags : 约定: assign(除了copy)后flags只有符号标志位, 其他写操作或懒求值操作均是修改flags的位
    Slice<std::uint32_t> m_mag{}; // 绝对值二进制, 去除前导零(高位 | 大下标)
    mutable SizeType m_firstNotZeroU32IndexLazy = 0; // 从低位开始数的第一个不为0的U32的下标
};

PGBN_NAMESPACE_END
#endif // !PGBIGNUMBER_BIGINTEGERIMPL_H