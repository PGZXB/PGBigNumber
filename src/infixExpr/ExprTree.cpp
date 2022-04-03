#include "ExprTree.h"

using namespace pgbn;
using namespace pgbn::infixExpr;

PGBN_INFIXEXPR_NAMESPACE_START namespace detail {

inline static Value eval(ExprNode * node, bool peval) {
    return peval ?
        node->valPromise.get_future().get() : // Wait
        node->evalCallback(node, false);
}

static Value ifElseNodeEvalCallback(ExprNode * node, bool peval) {
    PGZXB_DEBUG_ASSERT(node->children.size() == 3);
    auto & ch = node->children;
    return !eval(ch[0], peval).isZero() ? eval(ch[1], peval) : eval(ch[2], peval);
}

static Value binOpEvalCallback(ExprNode * node, bool peval) {
#define lOpAssRAndReturn(op) left.op##Assign(right); return left
    PGZXB_DEBUG_ASSERT(node->children.size() == 2);
    auto & ch = node->children;

    auto left = eval(ch[0], peval);
    auto right = eval(ch[1], peval);

    switch (node->nodeType) {
    case TokenType::OR :
        lOpAssRAndReturn(or);
    case TokenType::XOR :
        lOpAssRAndReturn(xor);
    case TokenType::AND :
        lOpAssRAndReturn(and);
    case TokenType::EQU :
        return Value(left.cmp(right) == 0);
    case TokenType::NEQU :
        return Value(left.cmp(right) != 0);
    case TokenType::LT :
        return Value(left.cmp(right) < 0);
    case TokenType::LE :
        return Value(left.cmp(right) <= 0);
    case TokenType::GT :
        return Value(left.cmp(right) > 0);
    case TokenType::GE :
        return Value(left.cmp(right) >= 0);
    case TokenType::ADD :
        lOpAssRAndReturn(add);
    case TokenType::SUB :
        lOpAssRAndReturn(sub);
    case TokenType::MUL :
        lOpAssRAndReturn(mul);
    case TokenType::DIV :
        lOpAssRAndReturn(div);
    case TokenType::SHIFTLEFT :
        left.shiftLeftAssign(right.toU64());
        return left;
    case TokenType::SHIFTRIGHT :
        left.shiftRightAssign(right.toU64());
        return left;
    case TokenType::POW :
        // TODO: Opt it
        if (right.isZero()) {
            return Value(0);
        }
        {
            auto res = Value(1);
            while (right.cmp(0) > 0) {
                res.mulAssign(left);
                right.dec();
            }
            return res;
        }
    default :
        PGZXB_DEBUG_ASSERT_EX("Can Not Be Reached", false);
    }
    return {};
#undef HELPER
}

static Value unaryOpEvalCallback(ExprNode * node, bool peval) {
    PGZXB_DEBUG_ASSERT(node->children.size() == 1);
    auto & ch = node->children[0];
    auto val = eval(ch, peval);

    switch(node->nodeType) {
    case TokenType::ADD : // +
        return val;
    case TokenType::SUB : // -
        val.negate(); return val;
    case TokenType::NOT : // ~
        val.notSelf(); return val;
    case TokenType::FACT : // !
        if (val.isZero()) return Value(1);
        if (val.flagsContains(BNFlag::NEGATIVE)) return Value(0);
        {
            Value res(1);
            while (!val.isOne()) {
                res.mulAssign(val);
                val.dec();
            }
            return res; 
        }
    default :
        PGZXB_DEBUG_ASSERT_EX("Can Not Be Reached", false);
    }
    return {};
}

static Value valEvalCallback(ExprNode * node, bool) {
    PGZXB_DEBUG_ASSERT(node->children.empty());
    if (node->nodeType == ExprNode::NodeType::LITERAL)
        return node->val;
    else if (node->nodeType == ExprNode::NodeType::BUILTINCONSTANT)
        return node->symbol->as<Value>();
    PGZXB_DEBUG_ASSERT_EX("Cannot Be Reached", false);
    return {};
}

static Value builtinFuncEvalCallback(ExprNode * node, bool peval) {
    PGZXB_DEBUG_ASSERT(node->nodeType == ExprNode::NodeType::BUILTINFUNC);
    Func func = node->symbol->as<Func>();
    if (func.argCount != -1 && static_cast<SizeType>(func.argCount) != node->children.size()) PGBN_GetGlobalStatus() = ErrCode::PARSE_INFIXEXPR_BUILTINFUNC_CALL_NOT_MATCH;
    PGZXB_DEBUG_ASSERT(!!func.nativeCall);
    std::vector<Value> args;
    for (auto * ch : node->children) { 
        args.push_back(eval(ch, peval));
    }
    return func.nativeCall(args);
}

static void pevalTask(ExprNode * node) {
    // std::ostringstream oss;
    // oss << std::this_thread::get_id();
    // std::cerr << pgfmt::format("Thread#{0} node#{1} start\n", oss.str(), (std::uint64_t)node );
    auto temp = node->evalCallback(node, true);
    node->valPromise.set_value(temp);
    // std::cerr << pgfmt::format("Thread#{0} node#{1}: peval = {2}\n", oss.str(),
    //     (std::uint64_t)node, temp.cmp(INT64_MAX) == 1 ? temp.toString(10).substr(0, 50).append("...") : temp.toString(10));
}

} PGBN_INFIXEXPR_NAMESPACE_END

using Node  = ExprNode;

// NodePool's public-funcitons
NodePool::NodePool() = default;

Node * NodePool::get(std::uint32_t index) {
    PGZXB_DEBUG_ASSERT_EX("index out of bound", index < m_count);
    if (index < INIT_BUF_SIZE)
        return &m_initBuffer[index];
    return &m_exBuffer[index - INIT_BUF_SIZE];
}

std::uint32_t NodePool::count() const {
    return m_count;
}

Node * NodePool::newNode() {
    Node * res = nullptr;
    if (m_count >= INIT_BUF_SIZE) {
        m_exBuffer.push_back(Node{});
        res = &m_exBuffer.back();
    } else {
        res = &m_initBuffer[m_count];
    }

    res->indexInPool = m_count++; // from 0
    res->pevalTask = detail::pevalTask;
    return res;
}

Node * NodePool::newIfElseNode(Node * cond, Node * then, Node * els) {
    Node * ptr = newNode();
    ptr->children = { cond, then, els };
    ptr->type = Node::Type::FUNCCALL_NODE;
    ptr->nodeType = Node::NodeType::QMARK;
    ptr->evalCallback = detail::ifElseNodeEvalCallback;
    return ptr;
}

Node * NodePool::newBinOpNode(Node::NodeType op, Node * lNode, Node * rNode) {
    Node * ptr = newNode();
    ptr->children = { lNode, rNode };
    ptr->type = Node::Type::FUNCCALL_NODE;
    ptr->nodeType = op;
    ptr->evalCallback = detail::binOpEvalCallback;
    return ptr;
}

Node * NodePool::newUnaryOpNode(Node::NodeType op, Node * child) {
    Node * ptr = newNode();
    ptr->children = { child };
    ptr->type = Node::Type::FUNCCALL_NODE;
    ptr->nodeType = op;
    ptr->evalCallback = detail::unaryOpEvalCallback;
    return ptr;
}

Node * NodePool::newLiteralNode(Value val) {
    Node * ptr = newNode();
    ptr->val = val;
    ptr->type = Node::Type::VAL_NODE;
    ptr->nodeType = Node::NodeType::LITERAL;
    ptr->evalCallback = detail::valEvalCallback;
    return ptr;
}

Node * NodePool::newBuiltinValNode(SymbolTable::Symbol * symbol) {
    Node * ptr = newNode();
    ptr->symbol = symbol;
    ptr->type = Node::Type::VAL_NODE;
    ptr->nodeType = Node::NodeType::BUILTINCONSTANT;
    ptr->evalCallback = detail::valEvalCallback;
    return ptr;
}

Node * NodePool::newFunctionCallNode(SymbolTable::Symbol * symbol, std::vector<Node*> argNodes) {
    Node * ptr = newNode();
    ptr->children = std::move(argNodes);
    ptr->symbol = symbol;
    ptr->type = Node::Type::FUNCCALL_NODE;
    ptr->nodeType = Node::NodeType::BUILTINFUNC;
    ptr->evalCallback = detail::builtinFuncEvalCallback;
    return ptr;
}

// NodePool's static-function
NodePool * NodePool::getDefaultInstance() {
    static NodePool ins;
    return &ins;
}
