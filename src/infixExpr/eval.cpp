#include "../expr.h"
#include "../streams/StringReadStream.h"
#include "../errInfos.h"
#include "ExprParser.h"
#include "Token.h"
#include "ExprTree.h"

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
