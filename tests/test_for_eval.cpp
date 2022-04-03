#include "expr.h"

#include <chrono>

using namespace pgbn;
using namespace pgbn::expr;

#undef PGZXB_DEBUG_INFO_HEADER
#define PGZXB_DEBUG_INFO_HEADER "[TEST]"

struct ScopedTimeCounter {
    std::size_t nanosec{0};
    const std::string name{nullptr};

    ScopedTimeCounter(const std::string & name) : name(name) {
        using std::chrono::system_clock;
        nanosec = system_clock::now().time_since_epoch().count();
    }
    
    ~ScopedTimeCounter() {
        using std::chrono::system_clock;
        nanosec = system_clock::now().time_since_epoch().count() - nanosec;
        std::cerr << pgfmt::format("{0} : {1}ns\n", name, nanosec);
    }
};

int main() {

    auto simpleExpr = "(30000! + 30000! + 30000! + 30000! + 30000! + 30000! + 30000! + 30000!) + (30000! + 30000! + 30000! + 30000! + 30000! + 30000! + 30000! + 30000!)";
    // auto simple = peval(simpleExpr);
    // PGZXB_DEBUG_ASSERT(simple == BigInteger(55));
    // PGZXB_DEBUG_Print(pgfmt::format("{0} = {1}\n", simpleExpr, simple.toString()));

    {
        ScopedTimeCounter _(pgfmt::format("peval {0}", simpleExpr));
        peval(simpleExpr);
    }
    {
        ScopedTimeCounter _(pgfmt::format(" eval {0}", simpleExpr));
        eval(simpleExpr);
    }

    std::string longAddExpr;
    constexpr int longAddExprOpValudCount = 2e9;


    return 0;
}
