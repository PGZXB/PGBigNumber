#include "fwd.h"

pgbn::infixExpr::Hooks * pgbn::infixExpr::Hooks::getInstance() {
    static thread_local Hooks ins;
    return &ins;
}
