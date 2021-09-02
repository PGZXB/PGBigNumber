#include "../include/PGBigNumber/Status.h"
#include "BigIntegerImpl.h"
#include "errInfos.h"

#include <charconv>
#include <limits>
#include <cstring>

using namespace pgbn;

// helper-functions

PGBN_NAMESPACE_START
namespace detail {

// 去除后导零
// static void eraseZerosPrefix(Slice<std::uint32_t> & s) { [useless]
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

// 比较Slice表示的正数的大小, less : -1, equals : 0, more : 1
static inline int cmpMag(const Slice<std::uint32_t> & a, const Slice<std::uint32_t> & b) {

    const SizeType alen = a.count();
    const SizeType blen = b.count();
    
    if (alen < blen) return -1;
    if (alen > blen) return +1;
    if (alen == 0) return 0;
    
    for (SizeType i = alen - 1; ; --i) {  // 从高位到低位比较
        std::uint32_t aa = a[i];
        std::uint32_t bb = b[i];

        if (aa != bb) return aa < bb ? -1 : +1;
        if (i == 0) break;
    }

    return 0;
}

// 将两个Slice表示的正整数本地相加, 结果回存到a
static inline void addMag(Slice<std::uint32_t> & a, const Slice<std::uint32_t> & b) {
    const SizeType maxlen = std::max(a.count(), b.count());
    const SizeType minlen = std::min(a.count(), b.count());

    const Slice<std::uint32_t> * temp = (maxlen == a.count()) ? &a : &b;

    a.slice(0, maxlen);

    constexpr std::uint64_t MASK = 0xffffffff;

    std::uint64_t sum = 0;
    SizeType i = 0;
    for (; i < minlen; ++i) {
        sum = (MASK & a[i]) + (MASK & b[i]) + (sum >> 32);
        a[i] = sum;
    }

    bool carry = ((sum >> 32) != 0);
    while (i < maxlen && carry) {
        a[i] = (*temp)[i] + 1;
        carry = (a[i] == 0);
        ++i;
    }

    if (carry) a.append(0x1);
    else if (i < maxlen && temp != &a) std::memcpy(&a[i], &(*temp)[i], (maxlen - i) * 4);
}

// 将两个Slice表示的正整数作差, 要求a > b, 结果将回存到a
static inline void subMag(Slice<std::uint32_t> & a, const Slice<std::uint32_t> & b) {
    PGZXB_DEBUG_ASSERT_EX("the len of a must be more than b's", a.count() >= b.count());
    const SizeType maxlen = a.count();
    const SizeType minlen = b.count();

    constexpr std::int64_t MASK = 0xffffffff;
    std::int64_t diff = 0;
    SizeType i = 0;
    for (; i < minlen; ++i) {
        PGZXB_DEBUG_ASSERT((diff >> 32) == 0 || (diff >> 32) == -1);
        diff =  (MASK & a[i]) - (MASK & b[i]) + (diff >> 32);
        a[i] = diff;
    }

    bool borrow = ((diff >> 32) != 0);
    while (i < maxlen && borrow)
        borrow = (--a[i++] == 0xffffffff);
}

// Slice与u32相乘, 本地算法
static inline void gradSchoolMulInplace(Slice<std::uint32_t> & a, std::uint32_t b) {
    const SizeType alen = a.count();
    PGZXB_DEBUG_ASSERT_EX("the size of a must be more than 0", alen > 0);

    constexpr std::uint64_t MASK = 0xffffffff;

    a.slice(0, alen);

    const std::uint64_t B = MASK & b;

    std::uint64_t carry = 0;
    for (SizeType i = 0; i < alen; ++i) {
        std::uint64_t temp = (MASK & a[i]) * B + carry;
        a[i] = temp;
        carry = (temp >> 32);    
    }

    if (carry != 0) a.append(carry);
}

// 两个Slice表示的正整数利用小学生算法相乘, a * b, 结果被返回
static inline Slice<std::uint32_t> gradeSchoolMul(
    const Slice<std::uint32_t> & a, const Slice<std::uint32_t> & b
) {
    const SizeType alen = a.count();
    const SizeType blen = b.count();
    PGZXB_DEBUG_ASSERT_EX("the size of a must be more than 0", alen > 0);
    PGZXB_DEBUG_ASSERT_EX("the size of b must be more than 0 too", blen > 0);

    constexpr std::uint64_t MASK = 0xffffffff;

    const SizeType rlen = alen + blen;
    Slice<std::uint32_t> result;
    result.slice(0, rlen);

    std::uint64_t carry = 0;
    for (SizeType i = 0; i < alen; ++i) {
        carry = 0;
        for (SizeType j = 0, k = i; j < blen; ++j, ++k) {
            std::uint64_t temp = (MASK & a[i]) * (MASK & b[j]) + (MASK & result[k]) + carry;
            result[k] = temp;
            carry = (temp >> 32);
        }
        result[i + blen] = carry;
    }

    return result;
}

// 将Slice表示的整数逻辑左移, 位不够则扩展, 本地算法
static inline void shlMag(Slice<std::uint32_t> & a, std::uint64_t n) {  // FIXME : can be better
    PGZXB_DEBUG_ASSERT_EX("the size of a must be more than 0", a.count() > 0);
    const SizeType nU32s = n / 32;
    const SizeType nBits = n & 0x1f; // n % 32

    if (nBits == 0) {
        PGZXB_DEBUG_ASSERT(n == nU32s * 32);
        a.preExtend(nU32s);
    } else {
        const SizeType nLastBits = 32 - nBits;
        const std::uint32_t temp = a[a.count() - 1] >> nLastBits;
        SizeType i;
        if (temp == 0) {
            a.slice(0, a.count() + nU32s);
            i = a.count() - 1;
        } else {
            a.slice(0, a.count() + nU32s + 1);
            a[a.count() - 1] = temp;
            i = a.count() - 2;
        }
        for (; i > nU32s; --i) {
            a[i] = (a[i - nU32s] << nBits) | (a[i - nU32s - 1] >> nLastBits);
        }
        a[i] = a[0] << nBits;
        std::fill_n(a.begin(), nU32s, 0); // 低位补零
    }
}

// 将Slice表示的整数逻辑右移, 位不够则扩展, 本地算法
static inline void shrMag(Slice<std::uint32_t> & a, std::uint64_t n) {
    PGZXB_DEBUG_ASSERT_EX("the size of a must be more than 0", a.count() > 0);
    const SizeType nU32s = n / 32;
    const SizeType nBits = n & 0x1f; // n % 32

    if (nU32s < a.count()) a.slice(nU32s, a.count());
    else { a.slice(0, 0); return; }

    if (nBits == 0) {
        PGZXB_DEBUG_ASSERT(n == nU32s * 32);
        return;
    } else {
        const SizeType nLastBits = 32 - nBits;
        const SizeType lMinu1 = a.count() - 1;
        SizeType i = 0;
        for (; i < lMinu1; ++i) {
            a[i] >>= nBits;
            a[i] |= (a[i + 1] << nLastBits);
        }
        const std::uint32_t temp = a[i] >> nBits;
        if (temp == 0) a.slice(0, a.count() - 1); // 去除前导零
        else {
            a[i] = temp;
        }
    }
}

// 将Slice表示的正整数自增
static inline void inc(Slice<std::uint32_t> & a) {
    PGZXB_DEBUG_ASSERT_EX("the size of a must be more than 0", a.count() > 0);

    const SizeType alen = a.count();
    SizeType i = 0;
    bool carry = true;
    while (i < alen && carry) {
        carry = (++a[i] == 0);
        ++i;
    }
    if (carry) {
        PGZXB_DEBUG_ASSERT(a[a.count() - 1] == 0);
        a.append(0x1);
    }
}

// 将Slice表示的正整数自减, a有可能变成0
static inline void dec(Slice<std::uint32_t> & a) {
    PGZXB_DEBUG_ASSERT_EX("the size of a must be more than 0", a.count() > 0);

    const SizeType alen = a.count();
    SizeType i = 0;
    bool borrow = true;
    while (i < alen && borrow) {
        borrow = (--a[i] == 0xffff'ffff);
        ++i;
    }

    // 由于要求了正数, 此时不可能存在borrow为true的情况
    PGZXB_DEBUG_ASSERT(!borrow);

    // 去除前导零
    eraseZerosSuffix(a);
}

// u32 -> string, 字符串是逆的, refer to http://www.strudel.org.uk/itoa/
static inline std::string u32toa_reversed(std::uint32_t value, int base) {
 
    std::string buf;

    // check that the base if valid
    if (base < 2 || base > 16) return buf;

    enum { kMaxDigits = 35 };
    buf.reserve( kMaxDigits ); // Pre-allocate enough space.

    std::uint32_t quotient = value;

    // Translating number to string with base:
    do {
        buf += "0123456789abcdef"[ quotient % base ];
        quotient /= base;
    } while ( quotient );

    // std::reverse( buf.begin(), buf.end() );
    return buf;
}

// 快速判断整数n是否为2的幂
static inline bool isPow2(std::uint64_t n) {
    return (n & (n - 1)) == 0;
}

// get bit-count from digit-count by radix
static inline SizeType getBitCountByDigitCount(SizeType numDigits, int radix) {
    static constexpr SizeType bitsPerDigit[] = { 
           0,    0, 1024, 1624, 2048, // 0~4
        2378, 2648, 2875, 3072, 3247, // 5~9
        3402, 3543, 3672, 3790, 3899, // 10~14
        4001, 4096, 4186, 4271, 4350, // 15~19
        4426, 4498, 4567, 4633, 4696, // 20~24
        4756, 4814, 4870, 4923, 4975, // 25~29
        5025, 5074, 5120, 5166, 5210, // 30~34
        5253, 5295                    // 35~36
    };
    constexpr int MIN_RADIX = 2;
    constexpr int MAX_RADIX = 36;
    PGZXB_DEBUG_ASSERT_EX("radix must in [2, 3,... ,36]", radix >= MIN_RADIX && radix <= MAX_RADIX);

    return ((numDigits * bitsPerDigit[radix]) >> 10) + 1;
}

// get digitsPerU32
static inline std::uint32_t getDigitCountPerU32(int radix) {
    // FIXME: Can be better
    static constexpr std::uint32_t digitsPerU32[] = {
        0 , 0 , 30, 19, 15, // 0~4
        13, 11, 11, 10, 9 , // 5~9
        9 , 8 , 8 , 8 , 8 , // 10~14 
        7 , 7 , 7 , 7 , 7 , // 15~19  
        7 , 7 , 6 , 6 , 6 , // 20~24
        6 , 6 , 6 , 6 , 6 , // 25~29  
        6 , 6 , 6 , 6 , 6 , // 30~34
        6 , 5               // 35~36
    };
    constexpr int MIN_RADIX = 2;
    constexpr int MAX_RADIX = 36;
    PGZXB_DEBUG_ASSERT_EX("radix must in [2, 3,... ,36]", radix >= MIN_RADIX && radix <= MAX_RADIX);

    return digitsPerU32[radix];
}

// get uint32 block radix
static inline std::uint32_t getU32BlockRadix(int radix) {
    // FIXME: Can be better
    static constexpr std::uint32_t blockRadix[] = {
        0x0       , 0x0       , 0x40000000, 0x4546b3db, 0x40000000, // 0~4
        0x48c27395, 0x159fd800, 0x75db9c97, 0x40000000, 0x17179149, // 5~9
        0x3b9aca00, 0xcc6db61 , 0x19a10000, 0x309f1021, 0x57f6c100, // 10~14
        0xa2f1b6f , 0x10000000, 0x18754571, 0x247dbc80, 0x3547667b, // 15~19
        0x4c4b4000, 0x6b5a6e1d, 0x6c20a40 ,  0x8d2d931,  0xb640000, // 20~24
        0xe8d4a51 , 0x1269ae40, 0x17179149, 0x1cb91000, 0x23744899, // 25~29
        0x2b73a840, 0x34e63b41, 0x40000000, 0x4cfa3cc1, 0x5c13d840, // 30~34
        0x6d91b519, 0x39aa400                                       // 35~36
    };
    constexpr int MIN_RADIX = 2;
    constexpr int MAX_RADIX = 36;
    PGZXB_DEBUG_ASSERT_EX("radix must in [2, 3,... ,36]", radix >= MIN_RADIX && radix <= MAX_RADIX);

    return blockRadix[radix];
}

// get u64 block radix
static inline const BigIntegerImpl & getU64BlockRadix(int radix) {
    using B = BigIntegerImpl;
    static const BigIntegerImpl u64BlockRadix[] = {
        B(0x0)               , B(0x0)               , B(0x4000000000000000), B(0x383d9170b85ff80b), B(0x4000000000000000), // 0~4
        B(0x6765c793fa10079d), B(0x41c21cb8e1000000), B(0x3642798750226111), B(0x1000000000000000), B(0x12bf307ae81ffd59), // 5~9
        B(0xde0b6b3a7640000 ), B(0x4d28cb56c33fa539), B(0x1eca170c00000000), B(0x780c7372621bd74d), B(0x1e39a5057d810000), // 10~14
        B(0x5b27ac993df97701), B(0x1000000000000000), B(0x27b95e997e21d9f1), B(0x5da0e1e53c5c8000), B(0xb16a458ef403f19 ), // 15~19
        B(0x16bcc41e90000000), B(0x2d04b7fdd9c0ef49), B(0x5658597bcaa24000), B(0x6feb266931a75b7 ), B(0xc29e98000000000 ), // 20~24
        B(0x14adf4b7320334b9), B(0x226ed36478bfa000), B(0x383d9170b85ff80b), B(0x5a3c23e39c000000), B(0x4e900abb53e6b71 ), // 25~29
        B(0x7600ec618141000 ), B(0xaee5720ee830681 ), B(0x1000000000000000), B(0x172588ad4f5f0981), B(0x211e44f7d02c1000), // 30~34
        B(0x2ee56725f06e5c71), B(0x41c21cb8e1000000)                                                                       // 35~36
    };
    constexpr int MIN_RADIX = 2;
    constexpr int MAX_RADIX = 36;
    PGZXB_DEBUG_ASSERT_EX("radix must in [2, 3,... ,36]", radix >= MIN_RADIX && radix <= MAX_RADIX);

    return u64BlockRadix[radix];
}

static inline const char * getZerosStr(int count) {
    PGZXB_DEBUG_ASSERT_EX("count must between 0 and 63", count >= 0 && count <= 63);
    static const char * ZEROS63 = "000000000000000000000000000000000000000000000000000000000000000"/*\0*/;
    return ZEROS63 + (63 - count);
}

static inline std::uint32_t getDigitCountPerU64(int radix) {
    static constexpr std::uint32_t digitCountPerU64[] = {
        0,  0, 62, 39, 31, // 0~4
        27, 24, 22, 20, 19, // 5~9
        18, 18, 17, 17, 16, // 10~14
        16, 15, 15, 15, 14, // 15~19
        14, 14, 14, 13, 13, // 20~24
        13, 13, 13, 13, 12, // 25~29
        12, 12, 12, 12, 12, // 30~34
        12, 12              // 35~36
    };

    constexpr int MIN_RADIX = 2;
    constexpr int MAX_RADIX = 36;
    PGZXB_DEBUG_ASSERT_EX("radix must in [2, 3,... ,36]", radix >= MIN_RADIX && radix <= MAX_RADIX);

    return digitCountPerU64[radix];
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

BigIntegerImpl::BigIntegerImpl(Slice<std::uint32_t> && slice, int signum) : m_mag(std::move(slice)) { // signum, 1 : posi, -1 : neg, 0 : zero
    switch (signum) {
    case -1 : setFlagsToNegative(); break;
    case 0 : beZero(); break;
    case 1 : setFlagsToPositive(); break;
    default : 
        PGZXB_DEBUG_ASSERT_EX("signum : -1 or 0 or +1", false);
        break;
    }

    detail::eraseZerosSuffix(m_mag); // 去除前导零
    if (m_mag.count() == 0) beZero();
}

BigIntegerImpl::BigIntegerImpl(const StringArg & str, int radix, bool * ok) {
    fromString(str, radix, ok);
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
    beginWrite(); // free-cache
    m_mag.cloneData(); // clone base-data

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

BigIntegerImpl & BigIntegerImpl::fromString(const StringArg & str, int radix, bool * ok) {
    constexpr int MIN_RADIX = 2;
    constexpr int MAX_RADIX = 36;
    if (radix < MIN_RADIX || radix > MAX_RADIX) {
        GetGlobalStatus() = ErrCode::RADIX_INVALID;
        ok && (*ok = false);
        return *this;
    }

    if (str.empty()) {
        GetGlobalStatus() = ErrCode::NUMBER_STRING_EMPTY;
        ok && (*ok = false);
        return *this;
    }

    const SizeType slen = str.size();
    SizeType pos = 0;

    int sigNum = 1;
    auto posiIndex = str.find_last_of('+');
    auto negIndex = str.find_last_of('-');
    if (negIndex != StringArg::npos) {
        if (negIndex != 0 || posiIndex != StringArg::npos) { // 不是第0个或同时有+-都是非法
            GetGlobalStatus() = ErrCode::NUMBER_STRING_SIG_INVALID;
            ok && (*ok = false);
            return *this;
        }
        sigNum = -1;
        pos = 1;
    } else if (posiIndex != StringArg::npos) {
        if (posiIndex != 0) {
            GetGlobalStatus() = ErrCode::NUMBER_STRING_SIG_INVALID;
            ok && (*ok = false);
            return *this;
        }
        pos = 1;
    } else { // 没有+-号即为+
        sigNum = +1;
    }

    if (pos == slen) {
        GetGlobalStatus() = ErrCode::NUMBER_STRING_EMPTY;
        ok && (*ok = false);
        return *this;
    }

    pos = str.find_first_not_of('0', pos);
    if (pos == StringArg::npos) { // 没有非0位, 就是0
        beZero();
        return *this;
    }

    BigIntegerImpl temp;
    temp.setFlagsToPositive();

    SizeType digitCnt = slen - pos;
    SizeType bitCnt = detail::getBitCountByDigitCount(digitCnt, radix);
    if (bitCnt + 31 >= 0x1'0000'0000) {
        GetGlobalStatus() = ErrCode::ARITHMETIC_OVERFLOW;
        ok && (*ok = false);
        return *this;
    }
    // SizeType u32Cnt = (bitCnt + 31) / 32;
    const char * cStr = str.data(); // 为了构造"子串", 也许有更好且保持速度的方法
    
    SizeType fistBlockDigitCnt = digitCnt % detail:: getDigitCountPerU32(radix);
    if (fistBlockDigitCnt == 0) fistBlockDigitCnt = detail:: getDigitCountPerU32(radix);
    
    std::uint32_t mag0;
    auto fistCov2U32Result = std::from_chars(cStr + pos, cStr + pos + fistBlockDigitCnt, mag0, radix);
    if (fistCov2U32Result.ec != std::errc{}) { // to uint32_t出错
        GetGlobalStatus() = ErrCode::NUMBER_STRING_PARSE2NUM_ERROR;
        ok && (*ok = false);
        return *this;
    }
    pos += fistBlockDigitCnt;
    temp.m_mag.append(mag0);

    const SizeType blockSize = detail:: getDigitCountPerU32(radix);
    SizeType blockRadix = detail::getU32BlockRadix(radix);
    std::uint32_t blockValue = 0;
    while (pos < slen) {
        auto toU32Result = std::from_chars(cStr + pos, cStr + pos + blockSize, blockValue, radix);
        if (toU32Result.ec != std::errc{}) {
            GetGlobalStatus() = ErrCode::NUMBER_STRING_PARSE2NUM_ERROR;
            ok && (*ok = false);
            return *this;
        }
        pos += blockSize;
        temp.mulAssign(blockRadix);
        temp.addAssign(blockValue);
    }

    if (sigNum == -1) temp.setFlagsToNegative();
    else temp.setFlagsToPositive();
    assign(std::move(temp));
    GetGlobalStatus() = ErrCode::SUCCESS;
    ok && (*ok = true);
    return *this;
}

BigIntegerImpl & BigIntegerImpl::assign(const void * bin, SizeType len, bool little) {
    PGZXB_DEBUG_PTR_NON_NULL_CHECK(bin);
    beginWrite(); // free-cache
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
    if (m_mag.count() == 0) beZero();

    return *this;
}

void BigIntegerImpl::swap(BigIntegerImpl & other) noexcept {
    using std::swap;
    using pgbn::swap;
    swap(m_mag, other.m_mag);
    swap(m_flags, other.m_flags);
    swap(m_firstNotZeroU32IndexLazy, other.m_firstNotZeroU32IndexLazy);
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

std::string BigIntegerImpl::toString(int radix) const {
    constexpr int MIN_RADIX = 2;
    constexpr int MAX_RADIX = 36;
    PGZXB_DEBUG_ASSERT_EX("radix must in [2, 3,... ,36]", radix >= MIN_RADIX && radix <= MAX_RADIX);

    if (flagsContains(BNFlag::ZERO)) return "0";

    // /************************ 初步实现 *************************/
    // std::string res;
    // if (detail::isPow2(radix)) {
    //     if (radix == 8) {
    //         PGZXB_DEBUG_ASSERT_EX("Not Be Implemented!!", false);
    //     }
    //     std::uint32_t u32BitNumBaseRadixWhichIsPow2;
    //     const SizeType lenMinu1 = m_mag.count() - 1;
    //     switch (radix) {
    //     case 2 :
    //         u32BitNumBaseRadixWhichIsPow2 = 32; break;
    //     case 4 :
    //         u32BitNumBaseRadixWhichIsPow2 = 16; break;
    //     case 16 :
    //         u32BitNumBaseRadixWhichIsPow2 = 8; break;
    //     default :
    //         PGZXB_DEBUG_ASSERT_EX("Cannot Be Reached", false);
    //     }
    //     for (SizeType i = 0; i < lenMinu1; ++i) {
    //         auto temp = detail::u32toa_reversed(m_mag[i], radix);
    //         res.append(temp)
    //            .append(u32BitNumBaseRadixWhichIsPow2 - temp.size(), '0');
    //     }
    //     res.append(detail::u32toa_reversed(m_mag[lenMinu1], radix));
    //     if (flagsContains(BNFlag::NEGATIVE)) res.push_back('-');
    //     std::reverse(res.begin(), res.end());
    //     return res;
    // } else {
    //     PGZXB_DEBUG_ASSERT_EX("Not Be Implemented!!", false);
    // }
    // /***********************************************************/

    const SizeType digitCountPerU64 = detail::getDigitCountPerU64(radix);
    std::vector<std::string> strs;
    std::string res = flagsContains(BNFlag::NEGATIVE) ? "-" : "";
    strs.reserve((4 * m_mag.count() + 6) / 7);
    res.reserve(strs.size() * digitCountPerU64);
    char buf[65] = { 0 };

    const BigIntegerImpl & blockRadix = detail::getU64BlockRadix(radix);
    BigIntegerImpl r;
    BigIntegerImpl temp(*this);
    temp.abs();

    while (!temp.flagsContains(BNFlag::ZERO)) {
        temp.divideAssignAndReminder(blockRadix, r);
        std::uint64_t u64 = r.toU64();
        auto toCharsResult = std::to_chars(buf, buf + (sizeof(buf) / sizeof(*buf)), u64, radix);
        PGZXB_DEBUG_ASSERT(toCharsResult.ec == std::errc{});
        strs.emplace_back(buf, toCharsResult.ptr - buf);
    }

    res.append(strs.back());
    for (auto iter = strs.end() - 2, start = strs.begin(); iter >= start; --iter) {
        SizeType leadingZeroCount = digitCountPerU64 - iter->size();
        res.append(detail::getZerosStr(leadingZeroCount), leadingZeroCount)
           .append(*iter);
    }
    return res;
}

SizeType BigIntegerImpl::copyMagDataTo(void * dest, SizeType maxlen) const {
    if (flagsContains(BNFlag::ZERO)) return 0;

    const auto [bitCountR, bitCountQ] = getMagBitCount();
    const SizeType byteCount = util::ceilDivide(bitCountR, 8) + bitCountQ * 4;
    const SizeType len = std::min(maxlen, byteCount);

    if constexpr (isLittleEndian()) { // 无脑copy
        std::memcpy(dest, &m_mag[0], len);
    } else { // 稍作调整
        const SizeType u32Count = m_mag.count();
        PGZXB_DEBUG_ASSERT(u32Count > 0);
        std::uint32_t * destU32 = reinterpret_cast<std::uint32_t*>(dest);
        for (SizeType i = 0; i < u32Count - 1; ++i, ++destU32) {
            Byte * destByte = reinterpret_cast<Byte*>(destU32);
            const Byte * srcByte = reinterpret_cast<const Byte*>(&m_mag[i]);
            destByte[0] = srcByte[3];
            destByte[1] = srcByte[2];
            destByte[2] = srcByte[1];
            destByte[3] = srcByte[0];
        }

        Byte * destByte = reinterpret_cast<Byte*>(dest);
        destByte += (u32Count - 1) * 4;
        const Byte * srcByte = reinterpret_cast<const Byte*>(&m_mag[u32Count - 1]);
        if (byteCount % 4 != 0) {
            const SizeType remByteCount = byteCount % 4;
            for (SizeType i = 0; i < remByteCount; ++i) {
                destByte[i] = srcByte[3 - i];
            }
        } else {
            destByte[0] = srcByte[3];
            destByte[1] = srcByte[2];
            destByte[2] = srcByte[1];
            destByte[3] = srcByte[0];
        }
    }

    return len;
}

bool BigIntegerImpl::flagsContains(BNFLAG Enum flags) const {
    return (m_flags & flags) == flags;
}

bool BigIntegerImpl::flagsEquals(BNFLAG Enum flags) const {
    return m_flags == flags;
}

bool BigIntegerImpl::isOne() const {
    if (flagsContains(BNFlag::POSITIVE) && m_mag.count() == 1) {
        return m_mag[0] == 0x1;
    }

    return false;
}

bool BigIntegerImpl::isNegOne() const {
    if (flagsContains(BNFlag::NEGATIVE) && m_mag.count() == 1) {
        return m_mag[0] == 0x1;
    }

    return false;
}

bool BigIntegerImpl::isOdd() const {
    return !isEven();
}

bool BigIntegerImpl::isEven() const {
    if (flagsContains(BNFlag::ZERO)) return true;

    PGZXB_DEBUG_ASSERT(m_mag.count() > 0);
    std::uint32_t mag0 = m_mag[0];
    return (mag0 & 0x1) == 0;
}

void BigIntegerImpl::inc() {
    beginWrite(); // free-cache FIXME: May Be Better
    m_mag.cloneData(); // clone base-data

    if (flagsContains(BNFlag::ZERO)) { beOne(); return; } // 0自增为1

    if (flagsContains(BNFlag::POSITIVE)) { // 正数自增 -> mag自增 -> 为正数
        detail::inc(m_mag);
    } else { // 负数自增 -> mag自减 -> 为负数或0
        detail::dec(m_mag);
        if (m_mag.count() == 0) beZero();
    }
}

void BigIntegerImpl::dec() {
    beginWrite(); // free-cache FIXME: May Be Better
    m_mag.cloneData(); // clone base-data

    if (flagsContains(BNFlag::ZERO)) { beNegOne(); return; } // 0自减为-1

    if (flagsContains(BNFlag::POSITIVE)) { // 正数自减 -> mag自减 -> 为正数或0
        detail::dec(m_mag);
        if (m_mag.count() == 0) beZero();
    } else { // 负数自减 -> mag自增 -> 为负数
        detail::inc(m_mag);
    }
}

void BigIntegerImpl::addAssign(std::int64_t i64) {
    BigIntegerImpl temp(i64);

    addAssign(temp);
}

void BigIntegerImpl::addAssign(const BigIntegerImpl & other) {
    if (other.flagsContains(BNFlag::ZERO)) return;
    if (flagsContains(BNFlag::ZERO)) { assign(other); return; }

    beginWrite(); // 清除缓存

    if (hasSameSigFlag(other)) {
        m_mag.cloneData();
        detail::addMag(m_mag, other.m_mag);
        return;
    }

    int cmp = detail::cmpMag(m_mag, other.m_mag);

    if (cmp == 0) { beZero(); return; }

    if (cmp > 0) {
        m_mag.cloneData();
        detail::subMag(m_mag, other.m_mag);
    } else {
        Slice<std::uint32_t> copy = other.m_mag;
        copy.cloneData();
        detail::subMag(copy, m_mag);
        m_mag = copy;
    }

    detail::eraseZerosSuffix(m_mag);
    
    int sigNum = flagsContains(BNFlag::POSITIVE) ? +1 : (flagsContains(BNFlag::ZERO) ? 0 : -1);
    if (sigNum == cmp) setFlagsToPositive();
    else setFlagsToNegative();
}
    
void BigIntegerImpl::subAssign(std::int64_t i64) {
    BigIntegerImpl temp(i64);

    subAssign(temp);
}

void BigIntegerImpl::subAssign(const BigIntegerImpl & other) {
    if (other.flagsContains(BNFlag::ZERO)) return;
    if (flagsContains(BNFlag::ZERO)) {
        assign(other).negate();
        return;
    }

    if (this == &other) { beZero(); return; }
    if (hasSameSigFlag(other) && m_mag.weakEquals(other.m_mag)) { beZero(); return; }

    beginWrite();

    if (!hasSameSigFlag(other)) {
        m_mag.cloneData();
        detail::addMag(m_mag, other.m_mag);
        return;
    }

    int cmp = detail::cmpMag(m_mag, other.m_mag);

    if (cmp == 0) { beZero(); return; }

    if (cmp > 0) {
        m_mag.cloneData();
        detail::subMag(m_mag, other.m_mag);
    } else {
        Slice<std::uint32_t> copy = other.m_mag;
        copy.cloneData();
        detail::subMag(copy, m_mag);
        m_mag = copy;
    }

    detail::eraseZerosSuffix(m_mag);
    
    int sigNum = flagsContains(BNFlag::POSITIVE) ? +1 : (flagsContains(BNFlag::ZERO) ? 0 : -1);
    if (sigNum == cmp) setFlagsToPositive();
    else setFlagsToNegative();
}

void BigIntegerImpl::mulAssign(std::int64_t i64) {
    mulAssign(BigIntegerImpl(i64));
}

void BigIntegerImpl::mulAssign(const BigIntegerImpl & other) {
    if (flagsContains(BNFlag::ZERO) || other.flagsContains(BNFlag::ZERO)) { beZero(); return; }
    if (other.isOne()) return; // a * 1 == a
    if (other.isNegOne()) { negate(); return; } // a * -1 == -a
    if (isOne()) { assign(other); return; } // 1 * a == a
    if (isNegOne()) { assign(other).negate(); return; } // -1 * a == -a

    constexpr const SizeType KARATSUBA_THRESHOLD = 80;
    const SizeType alen = m_mag.count();
    const SizeType blen = other.m_mag.count();

    // free-cache
    beginWrite();

    // 规模较小直接采用小学生算法
    if (alen < KARATSUBA_THRESHOLD || blen < KARATSUBA_THRESHOLD) {
        // 设置符号位
        if (hasSameSigFlag(other)) setFlagsToPositive();
        else setFlagsToNegative();
        
        // 计算
        if (other.m_mag.count() == 1) {
            m_mag.cloneData();
            detail::gradSchoolMulInplace(m_mag, other.m_mag[0]);
            return;
        }
        if (m_mag.count() == 1) {
            std::uint32_t temp = m_mag[0];
            m_mag = other.m_mag;
            m_mag.cloneData();
            detail::gradSchoolMulInplace(m_mag, temp);
            return;
        }

        m_mag = detail::gradeSchoolMul(m_mag, other.m_mag);
        detail::eraseZerosSuffix(m_mag); // 去除前导零
        return;
    } else if (true) { // 规模中等采用Karatsuba算法
        BigIntegerImpl res;
        mulKaratsuba(res, *this, other);
        assign(res);
    }
    // 规模再大采用Toom Cook-3算法
}

void BigIntegerImpl::divAssign(std::int64_t i64) {
    divAssign(BigIntegerImpl(i64));
}

void BigIntegerImpl::divAssign(const BigIntegerImpl & other) {
    BigIntegerImpl r;
    BigIntegerImpl q;
    div(q, r, *this, other);
    assign(std::move(q));
}

void BigIntegerImpl::divideAssignAndReminder(const BigIntegerImpl & val, BigIntegerImpl & r) {
    BigIntegerImpl result;
    div(result, r, *this, val);
    assign(std::move(result));
}

void BigIntegerImpl::divideAssignAndReminder(std::int64_t i64, BigIntegerImpl & r) {
    BigIntegerImpl result;
    div(result, r, *this, BigIntegerImpl(i64));
    assign(std::move(result));
}

void BigIntegerImpl::modAssignAndQuotient(const BigIntegerImpl & val, BigIntegerImpl & q) {
    BigIntegerImpl result;
    div(q, result, *this, val);
    assign(std::move(result));
}

void BigIntegerImpl::modAssignAndQuotient(std::int64_t i64, BigIntegerImpl & q) {
    BigIntegerImpl result;
    div(q, result, *this, BigIntegerImpl(i64));
    assign(std::move(result));
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
        setFlagsToNegative(); \
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
        setFlagsToNegative(); // 负数
        beginWrite(); // free-cache
        detail::notPlusOne(m_mag, getFirstNonZeroU32Index()); // 补码转真值
        detail::eraseZerosSuffix(m_mag); // 去除前导零
        return;
    default :
        PGZXB_DEBUG_ASSERT_EX("Can Not Be Reached", false);
    } 
}

void BigIntegerImpl::notBits() {
    if (flagsContains(BNFlag::ZERO)) return;

    beginWrite(); // free-cache
    m_mag.cloneData(); // clone base-data

    const SizeType len = m_mag.count();
    for (SizeType i = 0; i < len - 1; ++i) {
        m_mag[i] = ~m_mag[i];
    }

    constexpr std::uint32_t MASK = 0xffff'ffff;

    auto & last = m_mag[len - 1];
    int leadingZeroCnt = util::clz(last);
    last = ~last & (MASK >> leadingZeroCnt);
}

void BigIntegerImpl::shiftLeftAssign(std::uint64_t u64) {
    if (flagsContains(BNFlag::ZERO)) return;

    if (u64 > 0) {
        beginWrite(); // free-cache // FIXME : can be better
        m_mag.cloneData(); // clone-data
        detail::shlMag(m_mag, u64);
        return;
    }
}

void BigIntegerImpl::shiftRightAssign(std::uint64_t u64) {
    if (flagsContains(BNFlag::ZERO)) return;

    if (u64 > 0) {
        if (u64 / 32ULL >= m_mag.count()) {
            if (flagsContains(BNFlag::NEGATIVE)) beNegOne();
            else beZero();
            return;
        }

        bool lostOne = false;
        if (flagsContains(BNFlag::NEGATIVE)) {
            // if (flagsContains(BNFlag::FIRST_NZ_U32_INDEX_CALCUED)) {
            //     SizeType willBeLowestIndex = u64 / 32ULL;
            //     auto fnzi = getFirstNonZeroU32Index();
            //     if (fnzi < willBeLowestIndex) lostOne = true;
            //     else if (fnzi > willBeLowestIndex) lostOne = false;
            //     else lostOne = ( m_mag[fnzi] << (32ULL - (u64 & 0x1f)) ) != 0;
            // } else {
            if (true) {
                SizeType i = 0;
                lostOne = false;
                for (const SizeType nU32 = u64 / 32ULL; i < nU32 && !lostOne; ++i)
                    lostOne = ( m_mag[i] != 0 );
                if (!lostOne && (u64 & 0x1f) != 0)
                    lostOne = ( (m_mag[i] << (32ULL - (u64 & 0x1f))) != 0 );
            }
        }

        beginWrite(); // free-cache // FIXME : can be better
        m_mag.cloneData(); // clone-data

        detail::shrMag(m_mag, u64);

        if (m_mag.count() == 0) {
            if (lostOne) { // lostOne隐含了flagsContains(BNFlag::NEGATIVE)
                PGZXB_DEBUG_ASSERT(flagsContains(BNFlag::NEGATIVE));
                beNegOne();
            } else beZero();
        } else if (lostOne) detail::inc(m_mag);

        return;
    }
}

void BigIntegerImpl::negate() {
    if (flagsContains(BNFlag::ZERO)) return;
    constexpr Enum POSI_NEG = BNFlag::POSITIVE | BNFlag::NEGATIVE;

    // beginWrite(); // 不需要清除缓存

    m_flags ^= POSI_NEG;
}

void BigIntegerImpl::abs() {
    if (flagsContains(BNFlag::NEGATIVE)) negate(); // 为负求相反数
    // else return; // 零或正无需处理
}

int BigIntegerImpl::cmp(const BigIntegerImpl & other) const {
    // less : -1, equals : 0, more : +1

    int selfSignum = flagsContains(BNFlag::POSITIVE) ? +1 : (flagsContains(BNFlag::ZERO) ? 0 : -1);
    int otherSignum = other.flagsContains(BNFlag::POSITIVE) ? +1 : (other.flagsContains(BNFlag::ZERO) ? 0 : -1);

    if (selfSignum < otherSignum) return -1; // 负 < 零/正, 零 < 正
    if (selfSignum > otherSignum) return +1; // 正 > 零/负, 零 > 负
    if (selfSignum == 0) return 0; // 零 == 零

    if (m_mag.weakEquals(other.m_mag)) return 0; // 底层mag数组相同

    int magCmp = detail::cmpMag(m_mag, other.m_mag);

    if (magCmp == 0) return 0; // 强相同

    // 正数, 绝对值越小越小; 负数, 绝对值越大值越小
    return selfSignum == 1 ? magCmp : -magCmp;
}

SizeType BigIntegerImpl::getMagU32Count() const {
    return m_mag.count();
}

std::tuple<SizeType, SizeType> BigIntegerImpl::getMagBitCount() const {
    if (flagsContains(BNFlag::ZERO)) return std::make_tuple(0, 0);
    PGZXB_DEBUG_ASSERT(m_mag.count() > 0);

    SizeType r, q;
    q = m_mag.count() - 1;
    
    std::uint32_t mlast = m_mag[q];
    int lzCount = ::pg::util::clz(mlast); // leading zero count
    int sbCount = 32 - lzCount; // significant bit count
    r = static_cast<SizeType>(sbCount);

    if (r == 32) { r = 0; ++q; }
   
    return std::make_tuple(r, q);
}

// BigIntegerImpl's static-functions
void BigIntegerImpl::mul(BigIntegerImpl & res, const BigIntegerImpl & a, const BigIntegerImpl & b) {
    if (a.flagsContains(BNFlag::ZERO) || b.flagsContains(BNFlag::ZERO)) { res.beZero(); return; }
    if (a.isOne()) { res.assign(b); return; } // b
    if (a.isNegOne()) { res.assign(b).negate(); return; } // -b
    if (b.isOne()) { res.assign(a); return; } // a
    if (b.isNegOne()) { res.assign(a).negate(); return; } // -a

    constexpr const SizeType KARATSUBA_THRESHOLD = 80;

    res.beginWrite();

    if (a.hasSameSigFlag(b)) res.setFlagsToPositive();
    else res.setFlagsToNegative();

    const SizeType alen = a.m_mag.count();
    const SizeType blen = b.m_mag.count();

    // 规模较小直接采用小学生算法
    if (alen < KARATSUBA_THRESHOLD || blen < KARATSUBA_THRESHOLD) {
        if (b.m_mag.count() == 1) {
            res.m_mag = a.m_mag;
            res.m_mag.cloneData();
            detail::gradSchoolMulInplace(res.m_mag, b.m_mag[0]);
            return;
        }
        if (a.m_mag.count() == 1) {
            res.m_mag = b.m_mag;
            res.m_mag.cloneData();
            detail::gradSchoolMulInplace(res.m_mag, a.m_mag[0]);
            return;
        }

        res.m_mag = detail::gradeSchoolMul(a.m_mag, b.m_mag);
        detail::eraseZerosSuffix(res.m_mag); // 去除前导零
        return;
    } else if (true) { // 规模中等采用Karatsuba算法
        mulKaratsuba(res, a, b);
    }
    // 规模再大采用Toom Cook-3算法
}

void BigIntegerImpl::div(BigIntegerImpl & q, BigIntegerImpl & r, const BigIntegerImpl & a, const BigIntegerImpl & b) {
    BigIntegerImpl dividend(a);
    BigIntegerImpl divisor(b);

    dividend.setFlagsToPositive(); // |a|
    divisor.setFlagsToPositive();  // |b|

    int aSigNum = a.flagsContains(BNFlag::POSITIVE) ? +1 : (a.flagsContains(BNFlag::ZERO) ? 0 : -1);
    int bSigNum = b.flagsContains(BNFlag::POSITIVE) ? +1 : (b.flagsContains(BNFlag::ZERO) ? 0 : -1);

    if (bSigNum == 0) { GetGlobalStatus() = ErrCode::DIVBY0; return; } // a/0 error
    if (aSigNum == 0) { q.beZero(); r.beZero(); return; } // 0/b == 0...0

    if (b.m_mag.count() == 1) { // 转发到 a/<int64_t>

        q.assign(a);
        q.divideAssignAndReminderByU32(b.m_mag[0], r);
        if (!q.flagsContains(BNFlag::ZERO)) {
            if (aSigNum == bSigNum) q.setFlagsToPositive();
            else q.setFlagsToNegative();
        }
        if (!r.flagsContains(BNFlag::ZERO)) {
            if (aSigNum < 0) r.setFlagsToNegative();
            else r.setFlagsToPositive();
        }
        return;
    }

    if (int cmpCode = dividend.cmp(divisor); cmpCode <= 0) { // |a| <= |b|
        if (cmpCode == 0) { // |a| == |b|
            if (aSigNum == bSigNum) q.beOne();
            else q.beNegOne();
            r.beZero();
        } else { // |a| < |b|
            q.beZero();
            r.assign(dividend);
            if (aSigNum > 0) r.setFlagsToPositive(); // 余数的符号和被除数的符号相同
            else r.setFlagsToNegative();
        }
        return;
    }

    PGZXB_DEBUG_ASSERT(dividend.m_mag.count() >= divisor.m_mag.count());
    dividend.beginWrite(); // free-cache
    dividend.m_mag.cloneData(); // clone-base-data
    if (dividend.m_mag.count() == divisor.m_mag.count()) dividend.m_mag.append(0);
    PGZXB_DEBUG_ASSERT(dividend.m_mag.count() > divisor.m_mag.count());

    // construct |q| and |r|
    knuthDivImpl(q, r, dividend, divisor);
    // set sig of q and r
    if (aSigNum == bSigNum) q.setFlagsToPositive();
    else q.setFlagsToNegative();
    if (!r.flagsContains(BNFlag::ZERO)) {
        if (aSigNum < 0) r.setFlagsToNegative();
        else r.setFlagsToPositive();
    }
    // set status
    GetGlobalStatus() = ErrCode::SUCCESS;
}

void BigIntegerImpl::div(BigIntegerImpl & q, BigIntegerImpl & r, const BigIntegerImpl & a, std::int64_t i64) {
    div(q, r, a, BigIntegerImpl(i64));
}

// BigIntegerImpl's private-functions
void BigIntegerImpl::mulKaratsuba(BigIntegerImpl & res, const BigIntegerImpl & a, const BigIntegerImpl & b) { // static-function
    const SizeType alen = a.m_mag.count();
    const SizeType blen = b.m_mag.count();

    const SizeType half = (std::max(alen, blen) + 1) / 2;

    std::vector<BigIntegerImpl> aBlocks = a.split(2, half); // {al, ah}
    std::vector<BigIntegerImpl> bBlocks = b.split(2, half); // {bl, bh}
    PGZXB_DEBUG_ASSERT(aBlocks.size() == 2);
    PGZXB_DEBUG_ASSERT(bBlocks.size() == 2);

    res.beginWrite();
    res.m_mag.cloneData();

    mul(res, aBlocks[1], bBlocks[1]); // res <- ah * bh, p1 = res

    BigIntegerImpl p2;
    mul(p2, aBlocks[0], bBlocks[0]); // p2 <- al * bl

    BigIntegerImpl p3;
    aBlocks[1].addAssign(aBlocks[0]);
    bBlocks[1].addAssign(bBlocks[0]);
    mul(p3, aBlocks[1], bBlocks[1]);

    p3.subAssign(res);
    p3.subAssign(p2);

    res.shiftLeftAssign(32 * half);
    res.addAssign(p3);
    res.shiftLeftAssign(32 * half);
    res.addAssign(p2);

    if (a.hasSameSigFlag(b)) res.setFlagsToPositive();
    else res.setFlagsToNegative();
}

void BigIntegerImpl::knuthDivImpl(BigIntegerImpl & q, BigIntegerImpl & r, const BigIntegerImpl & u, const BigIntegerImpl & v) {
    // refer to TAOCP II , Section 3.4.1, Written By Knuth

    // some useful contants
    constexpr std::uint64_t NEG1 = static_cast<std::uint64_t>(-1);

    // requires v.bits > 0, u.bits > v.bits
    PGZXB_DEBUG_ASSERT_EX("the bit-count of v must be more 1", v.m_mag.count() > 1);
    PGZXB_DEBUG_ASSERT_EX("the bit-count of u must be more than v's", u.m_mag.count() > v.m_mag.count());

    // copy u, v as dividend, divisor
    BigIntegerImpl dividend(u), divisor(v);

    // m, n
    const SizeType N = v.m_mag.count();
    const SizeType M = u.m_mag.count() - N;
    PGZXB_DEBUG_ASSERT(M > 0);

    // 规格化
    std::uint64_t b = 0x1'0000'0000; // base, 2 ** 32
    std::uint64_t lShift = 0; // logb(d)
    std::uint64_t vn1_copy = v.m_mag[N - 1]; // Vn-1
    while (vn1_copy < b / 2) { vn1_copy <<= 1; ++lShift; }
    dividend.shiftLeftAssign(lShift); // dividend <- dividend * d
    divisor.shiftLeftAssign(lShift);  // divisor <- divisor * d
    dividend.m_mag.append(0);
    auto & uMag = dividend.m_mag;
    const auto & vMag = divisor.m_mag;
    // PGZXB_DEBUG_ASSERT(uMag.count() == M + N + 1);
    PGZXB_DEBUG_ASSERT(vMag.count() == N);

    // create q-array
    Slice<std::uint32_t> qArr;
    qArr.slice(0, M + 1);

    // 初始化j, 开始loop
    const std::uint64_t vn1 = vMag[N - 1]; // Vn-1
    const std::uint64_t vn2 = vMag[N - 2]; // Vn-2
    for (SizeType j = M; j != NEG1; --j) {
        // 计算q的估计(qHat)
        const std::uint64_t ujn = uMag[j + N]; // Uj+n
        const std::uint64_t ujn1 = uMag[j + N - 1]; // Uj+n-1
        const std::uint64_t ujn2 = uMag[j + N - 2]; // Uj+n-2
        std::uint64_t qHat = (ujn * b + ujn1) / vn1; // q^ <- (Uj+n * b + Uj+n-1) / Vn-1
        std::uint64_t rHat = (ujn * b + ujn1) % vn1; // r^ <- (Uj+n * b + Uj+n-1) % Vn-1
        if (qHat == b || qHat * vn2 > b * rHat + ujn2) --qHat;
        // if (rHat < b && (qHat == b || qHat * vn2 > b * rHat + ujn2)) --qHat;
        
        // 乘和减
        std::int64_t borrow = 0;
        std::uint64_t carry = 0;
        for (SizeType k = 0; k < N; ++k) {
            carry += qHat * vMag[k];
            borrow += uMag[j + k];
            borrow -= static_cast<std::uint32_t>(carry);
            uMag[j + k] = static_cast<std::uint32_t>(borrow);
            borrow >>= 32;
            carry >>= 32;
        }
        borrow += uMag[j + N];
        borrow -= carry;
        uMag[j + N] = static_cast<std::uint32_t>(borrow);
        borrow >>= 32;

        // 测试余数
        qArr[j] = qHat;
        if (borrow < 0) {
            // 往回加
            while (borrow < 0) {
                PGZXB_DEBUG_ASSERT(borrow == -1);
                carry = 0;
                for (SizeType k = 0; k < N; ++k) {
                    carry += uMag[j + k];
                    carry += vMag[k];
                    uMag[j + k] = static_cast<std::uint32_t>(carry);
                    carry >>= 32;
                }
                carry += uMag[j + N];
                uMag[j + N] = static_cast<std::uint32_t>(carry);
                carry >>= 32;
                borrow += carry;
                --qHat;
            }
            PGZXB_DEBUG_ASSERT(borrow == 0);
        }
        qArr[j] = static_cast<std::uint32_t>(qHat);

        // 对j进行循环 j != -1, --j
    }

    // init result-q, result-r
    q.setFlagsToPositive();
    r.setFlagsToPositive();

    // 不(反)规格化 & Save Result
    detail::eraseZerosSuffix(qArr);
    q.m_mag = std::move(qArr); // save result q

    detail::eraseZerosSuffix(dividend.m_mag); // erase leading zeros
    dividend.shiftRightAssign(lShift); // 反规格化
    r.m_mag = std::move(dividend.m_mag); // save result r
    if (r.m_mag.count() == 0) r.beZero(); // if 0, let 0
}

void BigIntegerImpl::divideAssignAndReminderByU32(std::uint32_t u32, BigIntegerImpl & r) {
    if (u32 == 0) { GetGlobalStatus() = ErrCode::DIVBY0; return; }
    
    int thisSigNum = flagsContains(BNFlag::POSITIVE) ? +1 : (flagsContains(BNFlag::ZERO) ? 0 : -1);
    int u32SigNum = +1;
    if (flagsContains(BNFlag::ZERO)) {
        r.beZero();
        return;
    }
    if (u32 == +1) {
        r.beZero();
        if (thisSigNum == u32SigNum) this->setFlagsToPositive();
        else this->setFlagsToNegative();
        return;
    }

    constexpr std::uint64_t NEG1 = static_cast<std::uint64_t>(-1);

    beginWrite();
    m_mag.cloneData();
    auto & mag = m_mag;
    std::uint64_t x = u32;
    std::uint64_t y = 0;
    PGZXB_DEBUG_ASSERT(mag.count() > 0);
    for (SizeType i = mag.count() - 1; i != NEG1; --i) {
        y <<= 32;
        y += mag[i];
        mag[i] = static_cast<std::uint32_t>(y / x);
        y %= x;
    }
    
    // erase leading zeros
    detail::eraseZerosSuffix(mag);

    // set sig
    if (mag.count() == 0) beZero();
    else if (thisSigNum == u32SigNum) setFlagsToPositive();
    else setFlagsToNegative();

    // save r
    r.assign(y);
    if (!r.flagsContains(BNFlag::ZERO)) {
        if (thisSigNum > 0) r.setFlagsToPositive();
        else r.setFlagsToNegative();
    }
    GetGlobalStatus() = ErrCode::SUCCESS;
}

void BigIntegerImpl::modAssignAndQuotientByU32(std::uint32_t u32, BigIntegerImpl & q) {
    if (u32 == 0) { GetGlobalStatus() = ErrCode::DIVBY0; return; }
    
    int thisSigNum = flagsContains(BNFlag::POSITIVE) ? +1 : (flagsContains(BNFlag::ZERO) ? 0 : -1);
    int u32SigNum = +1;
    if (flagsContains(BNFlag::ZERO)) {
        q.assign(*this);
        beZero();
        return;
    }
    if (u32 == +1) {
        beZero();
        q.assign(*this);
        if (thisSigNum == u32SigNum) q.setFlagsToPositive();
        else q.setFlagsToNegative();
        return;
    }

    constexpr std::uint64_t NEG1 = static_cast<std::uint64_t>(-1);

    q.assign(*this);
    q.setFlagsToPositive();
    q.m_mag.cloneData();
    auto & mag = q.m_mag;
    std::uint64_t x = u32;
    std::uint64_t y = 0;

    PGZXB_DEBUG_ASSERT(mag.count() > 0);
    for (SizeType i = mag.count() - 1; i != NEG1; --i) {
        y <<= 32;
        y += mag[i];
        mag[i] = static_cast<std::uint32_t>(y / x);
        y %= x;
    }

    // erase leading zeros
    detail::eraseZerosSuffix(mag);

    // set sig
    if (thisSigNum == u32SigNum) q.setFlagsToPositive();
    else q.setFlagsToNegative();

    // ass
    assign(y);
    if (!flagsContains(BNFlag::ZERO)) {
        if (thisSigNum > 0) setFlagsToPositive();
        else setFlagsToNegative();
    }
    GetGlobalStatus() = ErrCode::SUCCESS;
}

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

void BigIntegerImpl::setFlagsToNegative() {
    detail::setBits(m_flags, BNFlag::NEGATIVE);
    detail::eraseBits(m_flags, BNFlag::POSITIVE);
    detail::eraseBits(m_flags, BNFlag::ZERO);
}

void BigIntegerImpl::setFlagsToZero() {
    detail::setBits(m_flags, BNFlag::ZERO);
    detail::eraseBits(m_flags, BNFlag::POSITIVE);
    detail::eraseBits(m_flags, BNFlag::NEGATIVE);
}

bool BigIntegerImpl::hasSameSigFlag(const BigIntegerImpl & other) const {
    constexpr Enum SIGNUM_BITS = BNFlag::ZERO | BNFlag::POSITIVE | BNFlag::NEGATIVE;

    return (SIGNUM_BITS & m_flags) == (SIGNUM_BITS & other.m_flags);
}

void BigIntegerImpl::beZero() {
    m_mag = Slice<std::uint32_t>();
    m_flags = BNFlag::ZERO;
    m_firstNotZeroU32IndexLazy = 0;
}

void BigIntegerImpl::beOne() {
    m_mag = Slice<std::uint32_t>();
    m_flags = BNFlag::POSITIVE;
    m_mag.append(0x1);
    m_firstNotZeroU32IndexLazy = 0;
    detail::setBits(m_flags, BNFlag::FIRST_NZ_U32_INDEX_CALCUED);
}

void BigIntegerImpl::beNegOne() {
    m_mag = Slice<std::uint32_t>();
    m_flags = BNFlag::NEGATIVE;
    m_mag.append(0x1);
    m_firstNotZeroU32IndexLazy = 0;
    detail::setBits(m_flags, BNFlag::FIRST_NZ_U32_INDEX_CALCUED);
}

std::uint64_t BigIntegerImpl::toU64() const {
    constexpr std::uint64_t MASK = 0xffff'ffff;
    return ((MASK & getU32(1)) << 32) | (MASK & getU32(0));
}

std::vector<BigIntegerImpl> BigIntegerImpl::split(SizeType n, SizeType size) const {
    PGZXB_DEBUG_ASSERT_EX("n * size must be more than m_mag.count()", n * size >= m_mag.count());
    std::vector<BigIntegerImpl> res;

    const SizeType numFull = m_mag.count() / size;
    for (SizeType i = 0; i < numFull; ++i) {
        res.emplace_back(m_mag.sliced(i * size, (i + 1) * size), +1);
    }

    if (m_mag.count() % size != 0) {
        res.emplace_back(m_mag.sliced(numFull * size, m_mag.count()), +1);
    }

    PGZXB_DEBUG_ASSERT(n >= res.size());
    const SizeType remNum = n - res.size();
    for (SizeType i = 0; i < remNum; ++i) {
        res.emplace_back();
        res.back().beZero();
    }

    PGZXB_DEBUG_ASSERT(n == res.size());
    
    return res;
}

// detail::BigIntegerImplToStringHelper::next's implement
std::tuple<SizeType, const char *, SizeType> detail::BigIntegerImplToStringHelper::next() {
    SizeType pos = 0;
    if (m_index == 0) {
        if (m_temp.flagsContains(BNFlag::ZERO)) return std::make_tuple(m_index++, "0", 1);
        else {
            if (m_temp.flagsContains(BNFlag::NEGATIVE)) m_buf[pos++] = '-';
            m_temp.abs();
        }
    }

    const BigIntegerImpl & blockRadix = detail::getU64BlockRadix(m_radix);
    BigIntegerImpl r;
    m_temp.divideAssignAndReminder(blockRadix, r);
    std::uint64_t u64 = r.toU64();

    auto toCharsResult = std::to_chars(m_buf + pos, m_buf + pos + (sizeof(m_buf) / sizeof(*m_buf)), u64, m_radix);
    PGZXB_DEBUG_ASSERT(toCharsResult.ec == std::errc{});

    return std::make_tuple(m_index++, m_buf, toCharsResult.ptr - m_buf);
}