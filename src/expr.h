#ifndef PGBIGNUMBER_EXPR_H
#define PGBIGNUMBER_EXPR_H

#include "fwd.h"
#include "BigInteger.h"
PGBN_EXPR_NAMESPACE_START
using Int = pgbn::BigInteger;

Int eval(const StringArg& expr, Status *status = nullptr);
Int peval(const StringArg& expr, int threads = -1, Status *status = nullptr);

PGBN_EXPR_NAMESPACE_END
#endif
