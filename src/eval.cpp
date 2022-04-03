#include "expr.h"
#include "streams/StringReadStream.h"
#include "infixExpr/Token.h"
#include "infixExpr/ExprTree.h"
#include "infixExpr/ExprParser.h"
#include "parallel/ThreadPool.h"

using namespace pgbn;
using namespace pg::base;
using namespace pgbn::expr;
using namespace pgbn::infixExpr;

static inline ExprNode * parseString(const StringArg &str, NodePool *nodePool) {
	StringReadStream testSrc1Stream(str.data());
	ExprParser parser(testSrc1Stream, *Hooks::getInstance(), nodePool);
	return parser.parse();
}

Int pgbn::expr::eval(const StringArg &expr, Status *status) {
	if (!status) status = Status::getInstance();
	NodePool nodePool;
	auto *root = parseString(expr, &nodePool);
	if (!root) {
		(*status) = ~ErrCode::SUCCESS; // FIXME: Temp
		return Int{};
	}
	return BigInteger(std::make_unique<BigIntegerImpl>(root->evalCallback(root, false)));
}

Int pgbn::expr::peval(const StringArg &expr, int threads, Status *status) {
	if (threads == 0) return eval(expr, status);
	if (threads < 0) threads = std::thread::hardware_concurrency();
	if (!status) status = Status::getInstance();
	
	NodePool nodePool;
	auto *root = parseString(expr, &nodePool);
	if (!root) {
		(*status) = ~ErrCode::SUCCESS; // FIXME: Temp
		return Int{};
	}

	// Topological sort tasks & Async run tasks
	parallel::ThreadPool threadPool(threads);
	const auto N = nodePool.count();
	std::queue<std::uint32_t> q;
	std::vector<std::uint32_t> parents(N);
	std::vector<std::uint32_t> childCnt(N);

	for (std::uint32_t i = 0; i < N; ++i) {
		auto * node = nodePool.get(i);
		const auto & children = node->children;
		if (children.size() == 0) q.push(i);
		childCnt[i] = children.size();
		for (const auto * child : children) {
			PGZXB_DEBUG_ASSERT(child->indexInPool < N);
			parents[child->indexInPool] = i;
		}
	}

	std::uint32_t cnt = 0;
	std::vector<std::future<void>> currLevelFu;
	while (cnt != N) {
		const std::uint32_t cntInQueue = q.size();
		for (std::uint32_t i = 0; i < cntInQueue; ++i, ++cnt) {
			auto childIndex = q.front();
			auto * child = nodePool.get(childIndex);
			q.pop();
			currLevelFu.push_back(threadPool.addTask(child->pevalTask, child));
			if (--childCnt[parents[childIndex]] == 0) {
				q.push(parents[childIndex]);
			}
		}
		for (auto &fu : currLevelFu) {
			fu.wait(); // Wait
		}
	}
	
	// Blocking get result
	return BigInteger(std::make_unique<BigIntegerImpl>(root->valPromise.get_future().get()));
}
