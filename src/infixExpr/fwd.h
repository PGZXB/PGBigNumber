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
    using Base::Base;

    using Base::operator=;

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

    ~Variant() {
        (*static_cast<Base*>(this)).~Base();
    }
};

using Value = Variant<pgbn::BigIntegerImpl, std::uint64_t>;

using UnaryFunc = std::function<Value(const Value &)>;
using BinaryFunc = std::function<Value(const Value &, const Value &)>;
using Func = Variant<UnaryFunc, BinaryFunc>;

PGBN_INFIXEXPR_NAMESPACE_END
#endif // !PGBIGNUMBER_INFIXEXPR_FWD_H