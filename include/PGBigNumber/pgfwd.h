//
// Created by PGZXB on 2021/6/11.
//

// my uniform fwd-header-file
//                     ---- PGZXB

#ifndef PGZXB_PGFWD_H
#define PGZXB_PGFWD_H

#include <cstdint>
#include <string>

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
#else
#define PGZXB_DEBUG_ASSERT_IF(x) PGZXB_PASS
#define PGZXB_DEBUG_ASSERT(exp) PGZXB_PASS
#define PGZXB_DEBUG_ASSERT_EX(msg, exp) PGZXB_PASS
#endif

PGZXB_ROOT_NAMESPACE_START

using SizeType = std::uint64_t;
using Enum = std::uint32_t;
using Byte = std::uint8_t;
using String = std::string;

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

}
PGZXB_ROOT_NAMESPACE_END
#endif // PGZXB_PGFWD_H
