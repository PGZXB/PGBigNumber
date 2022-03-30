#ifndef PGBIGNUMBER_INFIXEXPR_SYMBOLTABLE_H
#define PGBIGNUMBER_INFIXEXPR_SYMBOLTABLE_H

#include "fwd.h"
#include "ObjectManager.h"
PGBN_INFIXEXPR_NAMESPACE_START

namespace detail { using Symbol = Variant<std::monostate, Func, Value>; }

class SymbolTable : public ObjectManager<detail::Symbol> {
    using Base = ObjectManager<detail::Symbol>;
private:
    using Base::Base;
    using Base::operator=;

public:
    using Symbol = detail::Symbol;

    static SymbolTable * getInstance() {
        static SymbolTable ins;

        return &ins;
    }
};

PGBN_INFIXEXPR_NAMESPACE_END
#endif // !PGBIGNUMBER_INFIXEXPR_SYMBOLTABLE_H
