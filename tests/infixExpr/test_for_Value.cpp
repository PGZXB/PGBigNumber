#include <istream>

#ifndef PGZXB_DEBUG
    #define PGZXB_DEBUG
#endif

#ifndef PGBN_DEBUG
    #define PGBN_DEBUG
#endif

#include "infixExpr/fwd.h"

#undef PGZXB_DEBUG_INFO_HEADER
#define PGZXB_DEBUG_INFO_HEADER "[TEST] "

using namespace pg::util;
using namespace pgbn::infixExpr;

int main () {

    using TypeArrayTest = TypeArray
        <char, int, long, float, double, std::string, pg::Byte, pg::StringArg>;

    PGZXB_DEBUG_PrintVar(TypeArrayTest::index<char>());
    PGZXB_DEBUG_PrintVar(TypeArrayTest::index<int>());
    PGZXB_DEBUG_PrintVar(TypeArrayTest::index<long>());
    PGZXB_DEBUG_PrintVar(TypeArrayTest::index<float>());
    PGZXB_DEBUG_PrintVar(TypeArrayTest::index<double>());
    PGZXB_DEBUG_PrintVar(TypeArrayTest::index<std::string>());
    PGZXB_DEBUG_PrintVar(TypeArrayTest::index<pg::Byte>());
    PGZXB_DEBUG_PrintVar(TypeArrayTest::index<pg::StringArg>());
    PGZXB_DEBUG_PrintVar(TypeArrayTest::size());

    Variant<char, int, long, float, double, std::string> val;
    
    val = 'a';
    PGZXB_DEBUG_PrintVar(val.as<char>());
    PGZXB_DEBUG_PrintVar(val.is<char>());
    PGZXB_DEBUG_PrintVar(val.index());

    val = int(1);
    PGZXB_DEBUG_PrintVar(val.as<int>());
    PGZXB_DEBUG_PrintVar(val.is<int>());
    PGZXB_DEBUG_PrintVar(val.index());

    val = long(2);
    PGZXB_DEBUG_PrintVar(val.as<long>());
    PGZXB_DEBUG_PrintVar(val.is<long>());
    PGZXB_DEBUG_PrintVar(val.index());

    val = float(3.14);
    PGZXB_DEBUG_PrintVar(val.as<float>());
    PGZXB_DEBUG_PrintVar(val.is<float>());
    PGZXB_DEBUG_PrintVar(val.index());

    val = double(3.14159);
    PGZXB_DEBUG_PrintVar(val.as<double>());
    PGZXB_DEBUG_PrintVar(val.is<double>());
    PGZXB_DEBUG_PrintVar(val.index());

    val = std::string("Hello World");
    PGZXB_DEBUG_PrintVar(val.as<std::string>());
    PGZXB_DEBUG_PrintVar(val.is<std::string>());
    PGZXB_DEBUG_PrintVar(val.index());

    return 0;
}
