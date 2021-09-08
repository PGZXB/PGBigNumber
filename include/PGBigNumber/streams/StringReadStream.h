//
// Created by 42025 on 2021/3/6.
//
#ifndef PGZXB_STRINGREADSTREAM_H
#define PGZXB_STRINGREADSTREAM_H

#include "../pgfwd.h"
#include <cstring>
PGZXB_ROOT_NAMESPACE_START namespace base {

// only wrap char[], default no copy
class StringReadStream {
    static constexpr Enum InvalidFlag = 0U;
    static constexpr Enum STDStringFlag = 1U;
    static constexpr Enum CStrFlag = 1U << 2U;
    static constexpr Enum CopyFlag = 1U << 3U;

public:
    explicit StringReadStream(const char * cStr, bool copy = false) : m_flags(CStrFlag) {
        PGZXB_DEBUG_PTR_NON_NULL_CHECK(cStr);

        init(cStr, std::strlen(cStr), copy);
    }

    // template<SizeType LEN>
    // explicit StringReadStream(const char (&cStr)[LEN], bool copy = false) : m_flags(CStrFlag) {
    //     PGJSON_DEBUG_ASSERT_EX("LEN Should Be More Than 1", LEN > 1);
    
    //     init(cStr, LEN - 1, copy);
    // }

    explicit StringReadStream(const std::string & str, bool copy = false) : m_flags(STDStringFlag) {
        init(str.data(), str.size(), copy);
    }

    // only move, cannot copy
    StringReadStream(const StringReadStream &) = delete;
    StringReadStream & operator= (StringReadStream &) = delete;

    StringReadStream(StringReadStream && other) noexcept
    : m_flags(other.m_flags), m_cStr(other.m_cStr), m_current(other.m_current), m_end(other.m_end) {
        other.m_flags = InvalidFlag;
        other.m_cStr = nullptr;
        other.m_current = nullptr;
        other.m_end = nullptr;
    }

    StringReadStream & operator= (StringReadStream && other) noexcept {
        if (this == &other) return *this;

        this->~StringReadStream();

        m_flags = other.m_flags;
        m_cStr = other.m_cStr;
        m_current = other.m_current;
        m_end = other.m_end;

        other.m_flags = InvalidFlag;
        other.m_cStr = nullptr;
        other.m_current = nullptr;
        other.m_end = nullptr;

        return *this;
    }

    ~StringReadStream() {
        if (m_flags & CopyFlag) PGZXB_FREE(const_cast<char *>(m_cStr));
    }

    bool eof() const {
        return m_current == m_end;
    }

    char peek() const {
        return eof() ? 0 : *m_current;
    }

    char get() {
        return eof() ? 0 : *(m_current++);
    }

    SizeType tell() {
        return m_current - m_cStr;
    }
private:
    void init(const char * cStr, SizeType len, bool copy) {
        if (!copy) {
            m_cStr = m_current = cStr;
            m_end = m_cStr + len;
        } else {
            m_flags |= CopyFlag;
            char * ptr = reinterpret_cast<char*>(PGZXB_MALLOC((len + 1)));
            std::memcpy(ptr, cStr, (len + 1));
            m_cStr = m_current = ptr;
            m_end = m_cStr + len;
        }
    }
private:
    Enum m_flags = InvalidFlag;
    const char * m_cStr = nullptr;
    const char * m_current = nullptr;
    const char * m_end = nullptr;
};

} PGZXB_ROOT_NAMESPACE_END
#endif // !PGZXB_STRINGREADSTREAM_H
