#include "../../include/PGBigNumber/expr.h"
#include "../../include/PGBigNumber/streams/StringReadStream.h"
#include "../../src/infixExpr/ExprParser.h"
#include "../../src/infixExpr/Token.h"
#include "../../src/infixExpr/ExprTree.h"
#include "../../src/errInfos.h"

using namespace pgbn;
using namespace pg::base;
using namespace pgbn::expr;
using namespace pgbn::infixExpr;

Int pgbn::expr::eval(const StringArg &expr, Status *status) {
	if (!status) status = Status::getInstance();
	StringReadStream testSrc1Stream(expr.data());
	ExprParser parser(testSrc1Stream, *Hooks::getInstance(), NodePool::getDefaultInstance());
	auto *root = parser.parse();
	if (!root) {
		(*status) = ~ErrCode::SUCCESS;
		return Int{};
	}
	return BigInteger(std::make_unique<BigIntegerImpl>(root->evalCallback(root)));
}
