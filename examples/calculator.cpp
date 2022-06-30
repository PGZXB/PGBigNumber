#include <iostream>
#include <string>
#include "PGBigNumber/expr.h"
#include "PGBigNumber/errInfos.h"
#include "PGBigNumber/Status.h"
#include "PGBigNumber/infixExpr/SymbolTable.h"

using pgbn::Status;
using pgbn::expr::eval;
using namespace pgbn::infixExpr;

int main () {
    std::setbuf(stdout, NULL);

    auto pST = SymbolTable::getInstance();
    pST->registe("sum", SymbolTable::Symbol{Func{
    /*.nativeCall = */ [](std::vector<Value> &args) -> Value {
        Value res;
        for (auto & e : args) {
            res.addAssign(e);
        }
        return res;
    },
    /*.argCount =*/ -1
    }});
    pST->registe("max", SymbolTable::Symbol{Func{
    /*.nativeCall = */ [](std::vector<Value> &args) -> Value {
        if (args.empty()) return {};
        Value res = args[0];
        for (auto & e : args) {
            if (res.cmp(e) == -1) {
                res = e;
            }
        }
        return res;
    },
    /*.argCount =*/ -1
    }});

    while (true) {
        Status status;
        std::string expr;
        std::cout << "> ";
        std::getline(std::cin, expr);
        auto result = eval(expr, &status);
        std::cout << pgfmt::format("{0} = {1}\n", expr, result.toString());
    }

    return 0;
}
