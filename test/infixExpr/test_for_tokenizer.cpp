#undef PGBN_DEBUG
#define PGBN_DEBUG
#include "../../src/BigIntegerImpl.h"
#include "../../src/infixExpr/Token.h"
#include "../../include/PGBigNumber/streams/StringReadStream.h"

using namespace pgbn::infixExpr;

namespace pg::util::stringUtil::__IN_fmtUtil {
    template<>
    inline std::string transToString<Token>(const Token & token, const std::string &) {
        return "\n\"[" + std::to_string(static_cast<Enum>(token.type)) + "]" + token.debugInfo + "\"";
    }
}

int main () {

    auto pST = SymbolTable::getInstance();
    pST->registe("sum", SymbolTable::Symbol{Func{BinaryFunc{}}});
    pST->registe("min", SymbolTable::Symbol{Func{BinaryFunc{}}});
    pST->registe("_100PI", SymbolTable::Symbol{Value{314}});

    const char testSrc1[] = 
        "-19412734219.8479123791 + min(987382 ** 2 >> (2!), sum(12344, -02x110100)) + _100PI + ((0 - 0x2d123ab3ff2) * (012x3213113 / 08x2010101)) + 1 & 2 | 9 + ~2123";

    pg::base::StringReadStream testSrc1Stream(testSrc1);
    auto testSrc1Tokens = tokenizer(testSrc1Stream);

    std::cout << pgfmt::format("[TEST]{0}\n", testSrc1Tokens);

    return 0;
}