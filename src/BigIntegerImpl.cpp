#include "BigIntegerImpl.h"

#include <limits>
#include <cstring>

using namespace pgbn;

// helper-functions
// static void eraseZerosPrefix(Slice<std::uint32_t> & s) {
//     const SizeType cnt = s.count();
//     SizeType i = 0;
//     while (i < cnt && s[i] == 0) ++i;
//     s.slice(i, cnt); 
// }

static void eraseZerosSuffix(Slice<std::uint32_t> & s) {
    auto iter = s.end() - 1;
    while (iter >= s.begin() && *iter == 0) --iter;
    s.slice(0, iter - s.begin() + 1);
}

// BigIntegerImpl's member functions
BigIntegerImpl::BigIntegerImpl() = default;

BigIntegerImpl::BigIntegerImpl(const BigIntegerImpl & other) = default; // 浅拷贝

BigIntegerImpl::BigIntegerImpl(BigIntegerImpl && other) noexcept = default; // 浅拷贝

BigIntegerImpl::BigIntegerImpl(std::int64_t i64) {
    assign(i64);
}

BigIntegerImpl::BigIntegerImpl(const void * bin, SizeType len, bool little) {
    assign(bin, len, little);
}

BigIntegerImpl & BigIntegerImpl::assign(const BigIntegerImpl & other) { // 浅拷贝
    if (this == &other) return *this;

    m_flags = other.m_flags;
    m_mag = other.m_mag;
    
    return *this;
}

BigIntegerImpl & BigIntegerImpl::assign(BigIntegerImpl && other) {
    if (this == &other) return *this;

    m_flags = other.m_flags;
    m_mag = std::move(other.m_mag);

    other.m_flags = BNFlag::INVALID;
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
        const Byte sign = m_flags == BNFlag::POSITIVE ? 0 : 0xff;
        
        while (bytes >= start && *bytes == sign) --bytes;
        len =  (bytes + 1) - start;
    } else {
        m_flags = reinterpret_cast<const std::int8_t *>(bin)[0] < 0 ? BNFlag::NEGATIVE : BNFlag::POSITIVE;
        
        const Byte * bytes = reinterpret_cast<const Byte *>(bin);
        const Byte * start = reinterpret_cast<const Byte *>(bin);
        const Byte * end = reinterpret_cast<const Byte *>(bin) + len;
        const Byte sign = m_flags == BNFlag::POSITIVE ? 0 : 0xff;
        
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
    
    eraseZerosSuffix(m_mag); // 去除前导零
    if (m_mag.count() == 0) m_flags |= BNFlag::ZERO;

    return *this;
}