//
// Created by PGZXB on 2021/6/11.
//

// my uniform fwd-header-file
//                     ---- PGZXB

#ifndef PGZXB_PGFWD_H
#define PGZXB_PGFWD_H

#include <cstdint>
#include <string>

#ifdef _MSC_VER
#include <intrin.h>
#endif

#define PGZXB_ROOT_NAMESPACE_START namespace pg {
#define PGZXB_ROOT_NAMESPACE_END }
#define PGZXB_PASS (void(0))
#define PGZXB_MIN_CPP_VERSION 201103L

#define PGZXB_MALLOC(size) malloc(size)
#define PGZXB_REALLOC(ptr, newSize) realloc((ptr), (newSize))
#define PGZXB_FREE(ptr) free(ptr)

#define PGZXB_STATIC_ASSERT_EX(msg, exp) static_assert(exp, msg)
#define PGZXB_STATIC_ASSERT(exp) PGZXB_STATIC_ASSERT_EX("", exp)

#if defined(PGZXB_DEBUG)

    #include <cstdio>
    #include <cassert>

    #define PGZXB_ASSERT_IF(x) for ( ; !(x) ; )

    #define PGZXB_DEBUG_ASSERT(exp) \
        PGZXB_ASSERT_IF(exp) { assert(exp); break; } \
        PGZXB_PASS

    #define PGZXB_DEBUG_ASSERT_EX(msg, exp) \
        PGZXB_ASSERT_IF((exp)) { \
            std::fputs(msg, stderr); \
            std::fputs(" : \n", stderr); \
            assert(exp); \
            break; \
        } PGZXB_PASS

    #define PGZXB_DEBUG_PTR_NON_NULL_CHECK(ptr) PGZXB_DEBUG_ASSERT_EX("Pointer Not Null", (ptr) != nullptr)

    #define PGZXB_DEBUG_EXEC(code) code

#else
    #define PGZXB_DEBUG_ASSERT_IF(x) PGZXB_PASS
    #define PGZXB_DEBUG_ASSERT(exp) PGZXB_PASS
    #define PGZXB_DEBUG_ASSERT_EX(msg, exp) PGZXB_PASS
    #define PGZXB_DEBUG_PTR_NON_NULL_CHECK(ptr) PGZXB_PASS
    #define PGZXB_DEBUG_EXEC(code) PGZXB_PASS
#endif

PGZXB_ROOT_NAMESPACE_START

using SizeType = std::uint64_t;
using Enum = std::uint32_t;
using Byte = std::uint8_t;
using String = std::string;

#define PRI_ENUM PRIu32

constexpr SizeType DEFAULT_BUFFER_SIZE = 512;

#if __cplusplus >= 201703L
    #include <string_view>
    using StringArg = std::string_view;
#else
    using StringArg = std::string;  // FIXME : replace with PGBase pg::base::StringArg
#endif

template <typename TYPE, typename ... ARGS>
inline TYPE * PGZXB_NEW(ARGS ... args) {
    return new TYPE(std::forward<ARGS>(args)...);
}

template <typename TYPE>
inline void PGZXB_DELETE(TYPE * ptr) {
    delete ptr;
}

template <typename TYPE>
inline TYPE * PGZXB_BATCH_NEW(SizeType count) {
    return new TYPE[count];
}

template <typename TYPE>
inline void PGZXB_BATCH_DELETE(TYPE * ptr) {
    delete [] ptr;
}

namespace util {

template <typename TYPE>
struct Deleter {
    void operator() (TYPE * ptr) const {
        pg::PGZXB_DELETE(ptr);
    }
};

#if defined(__GNUC__)
int inline popcnt(std::uint32_t x) {
    return __builtin_popcount(x);
}
#else
int inline popcnt(std::uint32_t x) {
    x -= ((x >> 1) & 0x55555555);
    x = (((x >> 2) & 0x33333333) + (x & 0x33333333));
    x = (((x >> 4) + x) & 0x0f0f0f0f);
    x += (x >> 8);
    x += (x >> 16);
    return (int)(std::uint32_t)(x & 0x0000003f);
}
#endif

#if defined(__GNUC__)
int inline clz(std::uint32_t x) {
    return __builtin_clz(x);
}

int inline ctz(std::uint32_t x) {
    return __builtin_ctz(x);
}
#elif defined(_MSC_VER)
int __inline ctz(std::uint32_t value) {
    unsigned long trailing_zero = 0;

    if (_BitScanForward(&trailing_zero, value)) return (int)(std::uint32_t)trailing_zero;
    else return 32;
}

int __inline clz(std::uint32_t value) {
    unsigned long leading_zero = 0;

    if (_BitScanReverse( &leading_zero, value )) return 31 - leading_zero;
    else return 32;
}
#else
int inline clz(std::uint32_t x) {
    x |= (x >> 1);
    x |= (x >> 2);
    x |= (x >> 4);
    x |= (x >> 8);
    x |= (x >> 16);
    return (int)(std::uint32_t)(32 - popcnt(x));
}
int inline ctz(std::uint32_t x) {
    return popcnt((x & -x) - 1);
}
#endif

template<typename A, typename B>
inline auto ceilDivide(const A & dividend, const B & divisor) {
    return dividend / divisor + (dividend % divisor != 0);
}

inline std::uint64_t cStrToU64(const char * str, char ** str_end, int radix) {
    return std::strtoull(str, str_end, radix);
}

inline std::uint32_t cStrToU32(const char * str, char ** str_end, int radix) {
    errno = 0;
    std::uint64_t result = cStrToU64(str, str_end, radix);
    if (errno != 0 || // 出错或者大于u32最大值
        result > static_cast<std::uint64_t>(std::numeric_limits<std::uint32_t>::max())
    ) {
        errno = ERANGE;
        str_end && (*str_end = nullptr);
        return std::numeric_limits<std::uint32_t>::max();
    }
    return static_cast<std::uint32_t>(result);
}

namespace detail {

template <typename VISITOR> // VISITOR : void(SizeType, ARG);
inline void visitParamPackageHelper(VISITOR visitor, SizeType index) { }

template <typename VISITOR, typename ARG, typename ... ARGS>
inline void visitParamPackageHelper(VISITOR visitor, SizeType index, ARG arg, ARGS && ... args) {
    visitor(index, std::forward<ARG>(arg));
    visitParamPackageHelper(visitor, index + 1, std::forward<ARGS>(args)...);
}

}

template <typename VISITOR, typename ... ARGS>
inline void visitParamPackage(VISITOR visitor, ARGS && ... args) {
    detail::visitParamPackageHelper(visitor, 0, std::forward<ARGS>(args)...);
}

//template <typename TYPE, typename ... ARGS>
//inline std::shared_ptr<TYPE> makeShared(ARGS && ... args) {
//    using NCType = typename std::remove_const<TYPE>::type;
//    return std::shared_ptr<NCType>(PGZXB_NEW<NCType>(std::forward<ARGS>(args)...), PGZXB_DELETE);
//}

namespace detail {

struct TypeArrayInvalidDataType { };

struct TypeArrayBaseInvalidType {
    using Front = TypeArrayInvalidDataType;
    using Other = TypeArrayBaseInvalidType;

    static constexpr SizeType INDEX = static_cast<SizeType>(-1);
};

template <SizeType FINDEX, typename FIRST, typename ... OTHER>
struct TypeArrayBase {
    using Front = FIRST;
    using Other = TypeArrayBase<FINDEX + 1, OTHER...>;

    static constexpr SizeType INDEX = FINDEX;
};

template<SizeType FINDEX>
struct TypeArrayBase<FINDEX, TypeArrayInvalidDataType> {
    using Front = TypeArrayInvalidDataType;
    using Other = TypeArrayBaseInvalidType;

    static constexpr SizeType INDEX = FINDEX;
};

}

template<typename ... TYPES>
struct TypeArray {
private:
    using Base = detail::TypeArrayBase<0, TYPES..., detail::TypeArrayInvalidDataType>;
    
public:
    static constexpr SizeType npos = static_cast<SizeType>(-1);

    template<typename T>
    static constexpr const SizeType index() {
        return indexImpl<T, Base>();
    }

    static constexpr const SizeType size() {
        return sizeof...(TYPES);
    }

private:
    template<typename T, typename TAB>
    static constexpr const SizeType indexImpl() {
        if constexpr (std::is_same_v<TAB, detail::TypeArrayBaseInvalidType>)
            return npos;

        if constexpr (std::is_same_v<T, typename TAB::Front>)
            return TAB::INDEX;

        return indexImpl<T, typename TAB::Other>();
    }
};

}
PGZXB_ROOT_NAMESPACE_END
#endif // PGZXB_PGFWD_H
