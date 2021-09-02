#undef PGBN_DEBUG
#define PGBN_DEBUG
#include "../../src/infixExpr/Token.h"
#include "../../include/PGBigNumber/streams/StringReadStream.h"

using namespace pgbn::infixExpr;

namespace pg::util::stringUtil::__IN_fmtUtil {
    template<>
    inline std::string transToString<Token>(const Token & token, const std::string &) {
        return "\n\t'\"[" + std::to_string(static_cast<Enum>(token.type)) + "]" + token.debugInfo + "\"";
    }
}

int main () {
    const char testSrc1[] = 
        "-19412734219.8479123791 + sin_bigint(987382) + ((0 - 0x2d123ab3ff2) * (012x3213113 / 08x2010101))";


    pg::base::StringReadStream testSrc1Stream(testSrc1);
    auto testSrc1Tokens = tokenizer(testSrc1Stream);
    std::cout << pgfmt::format("[TEST]{0}\n", testSrc1Tokens);

    return 0;
}