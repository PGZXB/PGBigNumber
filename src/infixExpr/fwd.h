#ifndef PGBIGNUMBER_INFIXEXPR_FWD_H
#define PGBIGNUMBER_INFIXEXPR_FWD_H

/**
 * 
 * 将中缀表达式转换为BigNumber.??
 *      ??可能是BigInteger(Impl), Fraction(Impl)或Union{ BigInteger(Impl), Fraction(Impl) }
 * 
 * 支持的运算 : 
 *      +, -, *, /, !(fact), **(pow), &(and), |(or), ^(xor), ~(not), >>(shift-right), <<(shift-left)
 *      支持内置函数(留有注册接口), 支持内置常量(留有注册接口)
 * 
 * 基本思路 : 按照编译技术的通用步骤, 此举主要是为了给CigTha项目探路, 不追求高效率
 * 
 * Value的格式 : (0 | ( (0[2-36]?x)?[1-9a-zA-Z][0-9a-zA-Z]* ) )(.[0-9a-zA-Z]+)?
 *      <正负号><数字>.<数字>e<整数>
 * 
 */
#include "../../include/PGBigNumber/fwd.h"
#include "../../include/PGBigNumber/Status.h"
#include "../BigIntegerImpl.h"
#include <variant>

#define PGBN_INFIXEXPR_NAMESPACE_START PGBN_NAMESPACE_START namespace infixExpr {
#define PGBN_INFIXEXPR_NAMESPACE_END } PGBN_NAMESPACE_END

PGBN_INFIXEXPR_NAMESPACE_START

template<typename ... TYPES>
class Variant : public std::variant<TYPES...> {
    using Base = std::variant<TYPES...>;
    using Types = util::TypeArray<TYPES...>;
public:
    Variant() = default;
    Variant(const Variant &) = default;
    Variant(Variant &&) noexcept = default;

    template <typename T>
    Variant(T && t) : Base() {
        Base * baseThis = this;
        baseThis->template emplace<T>(std::forward<T>(t));
    }

    template <typename T>
    Variant & operator=(T && t) {
        Base * const baseThis = this;
        baseThis->template emplace<T>(std::forward<T>(t));
        return *this;
    }

    Variant & operator=(const Variant & other) {
        Base * baseThis = this;
        const Base & baseOther = other;
        baseThis->operator=(baseOther);
        return *this;
    }

    Variant & operator=(Variant && other) {
        Base * baseThis = this;
        Base & baseOther = other;
        baseThis->operator=(std::move(baseOther));
        return *this;
    }

    template<typename T>
    bool is() const {
        return Types::template index<T>() == this->index();
    }

    template<typename T>
    T & as() {
        return std::get<T>(*this);
    }

    template<typename T>
    const T & as() const {
        return std::get<T>(*this);
    }

    ~Variant() = default;
};

using Value = BigIntegerImpl;
struct Func {
    using ArgType = std::vector<Value> &;
    std::function<Value(ArgType)> nativeCall = nullptr;
    int argCount = 0; // -1 : dynamic arglist
};

struct Hooks {
    std::function<bool(Value &, int radix, const std::string &, const std::string &)> literal2Value = 
        [] (Value & result, int radix, const std::string & literalBeforeDot, const std::string &) -> bool {
            static_assert(std::is_same_v<Value, BigIntegerImpl>);
            bool ok = true;
            result.fromString(literalBeforeDot, radix, &ok);
            return ok;
        };

    static Hooks * getInstance();

private:
    Hooks() = default;
    Hooks(const Hooks &) = delete;
    Hooks(Hooks &&) noexcept = delete;

    Hooks & operator=(const Hooks &) = delete;
    Hooks & operator=(Hooks &&) noexcept = delete;
};

namespace detail {

template<typename STREAM>
void skipWhiteChar(STREAM & stream) {
    // ' ' '\n' '\r' '\t'
    char ch = stream.peek();
    while (ch == 0 || ch == ' ' || ch == '\r' || ch == '\n' || ch == '\t') {
        stream.get();
        ch = stream.peek();
    }
}

}

namespace utils {

template<typename STREAM>
void skipWhiteChar(STREAM & stream) {
    // ' ' '\n' '\r' '\t'
    char ch = stream.peek();
    while (ch == 0 || ch == ' ' || ch == '\r' || ch == '\n' || ch == '\t') {
        stream.get();
        ch = stream.peek();
    }
}

}

using OpCode = std::uint64_t;

PGBN_INFIXEXPR_NAMESPACE_END
#endif // !PGBIGNUMBER_INFIXEXPR_FWD_H