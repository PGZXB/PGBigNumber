#include "BigIntegerImpl.h"

#include <limits>
#include <cstring>

using namespace pgbn;

// helper-functions

PGBN_NAMESPACE_START
namespace detail {

// 去除后导零
// static void eraseZerosPrefix(Slice<std::uint32_t> & s) {
//     const SizeType cnt = s.count();
//     SizeType i = 0;
//     while (i < cnt && s[i] == 0) ++i;
//     s.slice(i, cnt); 
// }

// 去除前导零
static inline void eraseZerosSuffix(Slice<std::uint32_t> & s) {
    auto iter = s.end() - 1;
    while (iter >= s.begin() && *iter == 0) --iter;
    s.slice(0, iter - s.begin() + 1);
}

// 根据flags获取符号字节
static inline Byte signByte(BNFLAG Enum flags) {
    return (flags & BNFlag::NEGATIVE) != 0 ? 0xff : 0;
}

// 设置标志位
static inline void setBits(BNFLAG Enum & result, BNFLAG Enum bits) {
    result |= bits;
}

// 擦除标志位
static inline void eraseBits(BNFLAG Enum & result, BNFLAG Enum bits) {
    result &= ~bits;
}

// 把int64_t当做无限长的符号扩展后的整型, 获取第n个U32
static inline std::uint32_t getU32OfI64ByIndex(::std::int64_t i64, SizeType n) {
    if (i64 == 0) return 0x0; // 如果是零直接返回0
    if (n >= 2) return i64 < 0 ? 0xffffffff : 0x0; // 高于最高, 返回符号

    std::uint32_t u32 = static_cast<std::uint32_t>((i64 >> (n * 32ULL)) & (0x00000000ffffffff));

    return u32;
}

// 将二进制Slice取反加1
static inline void notPlusOne(Slice<std::uint32_t> & s, SizeType firstNonZeroU32Index) {
    const SizeType cnt = s.count();
    PGZXB_DEBUG_ASSERT(cnt != 0);
    PGZXB_DEBUG_ASSERT(firstNonZeroU32Index <= cnt);

    if (firstNonZeroU32Index == cnt) return;

    s[firstNonZeroU32Index] = -s[firstNonZeroU32Index];
    for (SizeType i = firstNonZeroU32Index + 1; i < cnt; ++i) {
        s[i] = ~s[i];
    }
}

}
PGBN_NAMESPACE_END

// BigIntegerImpl's member functions
BigIntegerImpl::BigIntegerImpl() = default;

BigIntegerImpl::BigIntegerImpl(const BigIntegerImpl &) = default; // 浅拷贝

BigIntegerImpl::BigIntegerImpl(BigIntegerImpl &&) noexcept = default; // 浅拷贝

BigIntegerImpl::BigIntegerImpl(std::int64_t i64) {
    assign(i64);
}

BigIntegerImpl::BigIntegerImpl(const void * bin, SizeType len, bool little) {
    assign(bin, len, little);
}

BigIntegerImpl & BigIntegerImpl::assign(const BigIntegerImpl & other) { // 浅拷贝
    if (this == &other) return *this;

    m_flags = other.m_flags;
    m_firstNotZeroU32IndexLazy = other.m_firstNotZeroU32IndexLazy;
    m_mag = other.m_mag;
    
    return *this;
}

BigIntegerImpl & BigIntegerImpl::assign(BigIntegerImpl && other) {
    if (this == &other) return *this;

    m_flags = other.m_flags;
    m_firstNotZeroU32IndexLazy = other.m_firstNotZeroU32IndexLazy;
    m_mag = std::move(other.m_mag);

    other.m_flags = BNFlag::INVALID;
    other.m_firstNotZeroU32IndexLazy = 0;
    return *this;
}

BigIntegerImpl & BigIntegerImpl::assign(std::int64_t i64) {
    m_mag.cloneData();

    if (i64 == 0) {
        m_flags = BNFlag::ZERO;
        m_mag.slice(0, 0);
        return *this;
    }

    m_flags = i64 > 0 ? BNFlag::POSITIVE : BNFlag::NEGATIVE;
    if (i64 < 0) i64 = -i64;

    std::uint64_t u64 = i64;

    int u32s = u64 < std::numeric_limits<std::uint32_t>::max() ? 1 : 2;
    m_mag.slice(0, u32s);

    std::uint32_t * p = reinterpret_cast<std::uint32_t*>(&u64);
    if constexpr (isLittleEndian()) { // 小端机
        m_mag[0] = p[0];
        if (u32s == 2) m_mag[1] = p[1];
    } else { // 大端机
        m_mag[0] = p[1];
        if (u32s == 2) m_mag[1] = p[0];
    }

    return *this;
}

BigIntegerImpl & BigIntegerImpl::assign(const void * bin, SizeType len, bool little) {
    PGZXB_DEBUG_PTR_NON_NULL_CHECK(bin);
    m_mag.cloneData(); // clone, copy on write

    if (len == 0) { // 为0直接为ZERO
        m_flags = BNFlag::ZERO;
        m_mag.slice(0, 0);
        return *this;
    }

    struct Temp { // RAII, to auto free memory
        ~Temp() { free(ptr); }
        void * ptr = nullptr;
    };

    // 设置正负标志位并去除前导符号位
    if (little) {
        m_flags = reinterpret_cast<const std::int8_t *>(bin)[len - 1] < 0 ? BNFlag::NEGATIVE : BNFlag::POSITIVE;
        
        const Byte * bytes = reinterpret_cast<const Byte *>(bin) + len - 1;
        const Byte * start = reinterpret_cast<const Byte *>(bin);
        const Byte sign = m_flags == BNFlag::POSITIVE ? 0 : 0xff; // FIXME : replace with signByte
        
        while (bytes >= start && *bytes == sign) --bytes;
        len =  (bytes + 1) - start;
    } else {
        m_flags = reinterpret_cast<const std::int8_t *>(bin)[0] < 0 ? BNFlag::NEGATIVE : BNFlag::POSITIVE;
        
        const Byte * bytes = reinterpret_cast<const Byte *>(bin);
        const Byte * start = reinterpret_cast<const Byte *>(bin);
        const Byte * end = reinterpret_cast<const Byte *>(bin) + len;
        const Byte sign = m_flags == BNFlag::POSITIVE ? 0 : 0xff; // FIXME : replace with signByte
        
        while (bytes < end && *bytes == sign) ++bytes;

        len = len - (bytes - start);
        bin = bytes;
    }
    // 为0或-1(二进制补码形式全为0或全为1)
    if (len == 0) {
        if (m_flags == BNFlag::NEGATIVE) { // -1
            len = 1;
            m_mag.slice(0, 1);
            m_mag[0] = 0x1;
        } else { // 0
            len = 0;
            m_flags = BNFlag::ZERO;
            m_mag.slice(0, 0);
        }
        return *this;
    }


    // 结合前面的计算为mag预分配空间
    SizeType u32s = len / 4 + (len % 4 != 0);
    const SizeType tlen = u32s * 4;

    m_mag.slice(0, u32s);
    std::memset(&m_mag[0], 0, tlen);
    struct Temp temp;

    // 根据机器和数据的大小端将数据转换成宏观小端的方式
    if constexpr (isLittleEndian()) {
        if (!little) {
            Byte * src = reinterpret_cast<Byte*>(std::malloc(len));
            std::memcpy(src, bin, len);
            std::reverse(src, src + len);
            temp.ptr = src;
            bin = src;
        }
    } else {
        if (!little) {
            std::uint32_t * src = reinterpret_cast<std::uint32_t*>(std::malloc(tlen));
            std::memset(src, 0, tlen - len);
            std::memcpy(src + (tlen - len), bin, len);
            std::reverse(src, src + u32s);
            temp.ptr = src;
            bin = src;
        } else {
            Byte * src = reinterpret_cast<Byte*>(std::malloc(tlen));
            std::memcpy(src, bin, len);
            std::memset(src + len, 0, tlen - len);
            std::reverse(src, src + tlen);
            std::uint32_t * src_u32 = reinterpret_cast<std::uint32_t*>(src);
            std::reverse(src_u32, src_u32 + u32s);
            temp.ptr = src_u32;
            bin = src_u32;
        }
        len = tlen;
    }

    /* 自此, 宏观上bin和m_mag都是小端 */ 

    // 若是负数则需补码转真值
    if (m_flags == BNFlag::NEGATIVE) {
        if (temp.ptr == nullptr) {
            temp.ptr = reinterpret_cast<Byte*>(std::malloc(len));
            std::memcpy(temp.ptr, bin, len);
        }
        Byte * bytes = reinterpret_cast<Byte*>(temp.ptr);
        SizeType i = 0;
        for (; i < len && bytes[i] == 0; ++i);

        if (i == len) { // - pow2
            m_mag.append(1);
            return *this;
        }          

        bytes[i] = -bytes[i];
        for (++i; i < len; ++i) bytes[i] = ~bytes[i];
        bin = bytes;
    }

    std::memcpy(&m_mag[0], bin, len);
    
    detail::eraseZerosSuffix(m_mag); // 去除前导零
    if (m_mag.count() == 0) m_flags |= BNFlag::ZERO;

    return *this;
}

std::uint32_t BigIntegerImpl::getU32(SizeType n) const {
    
    if (flagsContains(BNFlag::ZERO)) return 0x0; // 如果是零直接返回0
    if (n >= m_mag.count()) return flagsContains(BNFlag::NEGATIVE) ? 0xffffffff : 0x0; // 高于最高, 返回符号

    std::uint32_t magInt = m_mag[n];

    if (flagsContains(BNFlag::POSITIVE)) return magInt;
    
    auto fnu32zi = getFirstNonZeroU32Index();
    if (n > fnu32zi) return ~magInt;
    else if (n < fnu32zi) return 0x0;
    else return -magInt;
}

bool BigIntegerImpl::flagsContains(BNFLAG Enum flags) const {
    return (m_flags & flags) == flags;
}

bool BigIntegerImpl::flagsEquals(BNFLAG Enum flags) const {
    return m_flags == flags;
}

#define DEFINE_BITWISE(operator, otherlen, getU32Index) \
    m_mag.cloneData(); \
    getU32(0); \
    const SizeType len = std::max(m_mag.count(), otherlen) + 1; \
    m_mag.slice(0, len); \
    for (SizeType i = 0; i < len; ++i) { \
        m_mag[i] = getU32(i) operator getU32Index; \
    } \
    switch (m_mag[m_mag.count() - 1]) { \
    case 0 : \
        setFlagsToPositive(); \
        detail::eraseZerosSuffix(m_mag); \
        if (m_mag.count() == 0) setFlagsToZero(); \
        beginWrite(); \
        return; \
    case 0xffffffff : \
        setFlagsToNegtaive(); \
        beginWrite(); \
        detail::notPlusOne(m_mag, getFirstNonZeroU32Index()); \
        detail::eraseZerosSuffix(m_mag); \
        return; \
    default : \
        PGZXB_DEBUG_ASSERT_EX("Not Reached", false); \
    } PGZXB_PASS

#define DEFINE_BITWISE_WITH_I64(operator) \
    DEFINE_BITWISE(operator, static_cast<SizeType>(2), detail::getU32OfI64ByIndex(i64, i))

#define DEFINE_BITWISE_WITH_OTHER(operator) \
    DEFINE_BITWISE(operator, other.m_mag.count(), other.getU32(i))


void BigIntegerImpl::andAssign(std::int64_t i64) {
    DEFINE_BITWISE_WITH_I64(&);
}

void BigIntegerImpl::andAssign(const BigIntegerImpl & other) {
    DEFINE_BITWISE_WITH_OTHER(&);
}

void BigIntegerImpl::orAssign(std::int64_t i64) {
    DEFINE_BITWISE_WITH_I64(|);
}

void BigIntegerImpl::orAssign(const BigIntegerImpl & other) {
    DEFINE_BITWISE_WITH_OTHER(|);
}

void BigIntegerImpl::xorAssign(std::int64_t i64) {
    DEFINE_BITWISE_WITH_I64(^);
}

void BigIntegerImpl::xorAssign(const BigIntegerImpl & other) {
    DEFINE_BITWISE_WITH_OTHER(^);
}

void BigIntegerImpl::notSelf() {
    m_mag.cloneData(); // COW
    getU32(0); // cache

    const SizeType len = m_mag.count() + 1;
    m_mag.slice(0, len);
    for (SizeType i = 0; i < len; ++i) {
        m_mag[i] = ~getU32(i);
    }
    switch (m_mag[m_mag.count() - 1]) {
    case 0 :
        setFlagsToPositive(); // 正数
        detail::eraseZerosSuffix(m_mag); // 去除前导零
        if (m_mag.count() == 0) setFlagsToZero(); // 零
        beginWrite(); // free-cache
        return;
    case 0xffffffff :
        setFlagsToNegtaive(); // 负数
        beginWrite(); // free-cache
        detail::notPlusOne(m_mag, getFirstNonZeroU32Index()); // 补码转真值
        detail::eraseZerosSuffix(m_mag); // 去除前导零
        return;
    default :
        PGZXB_DEBUG_ASSERT_EX("Can Not Be Reached", false);
    } 
}

// BigIntegerImpl's private-functions
void BigIntegerImpl::beginWrite() {
    // 擦除所有的懒求值标志
    detail::eraseBits(m_flags, BNFlag::LAZY_CALCU_FLAGS);
}

SizeType BigIntegerImpl::getFirstNonZeroU32Index() const {

    // 如果求过且没被修改过直接返回
    if (flagsContains(BNFlag::FIRST_NZ_U32_INDEX_CALCUED))
        return m_firstNotZeroU32IndexLazy;

    // 重新求值并设置标志位
    auto iter = std::find_if(m_mag.begin(), m_mag.end(), [] (std::uint32_t e) { return e != 0; });
    
    detail::setBits(m_flags, BNFlag::FIRST_NZ_U32_INDEX_CALCUED);
    return m_firstNotZeroU32IndexLazy = (iter - m_mag.begin());
}

void BigIntegerImpl::setFlagsToPositive() {
    detail::setBits(m_flags, BNFlag::POSITIVE);
    detail::eraseBits(m_flags, BNFlag::NEGATIVE);
    detail::eraseBits(m_flags, BNFlag::ZERO);
}

void BigIntegerImpl::setFlagsToNegtaive() {
    detail::setBits(m_flags, BNFlag::NEGATIVE);
    detail::eraseBits(m_flags, BNFlag::POSITIVE);
    detail::eraseBits(m_flags, BNFlag::ZERO);
}

void BigIntegerImpl::setFlagsToZero() {
    detail::setBits(m_flags, BNFlag::ZERO);
    detail::eraseBits(m_flags, BNFlag::POSITIVE);
    detail::eraseBits(m_flags, BNFlag::NEGATIVE);
}

