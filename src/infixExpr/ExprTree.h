#ifndef PGBIGNUMBER_INFIXEXPR_EXPRTREE_H
#define PGBIGNUMBER_INFIXEXPR_EXPRTREE_H

#include <deque>
#include <future>

#include "fwd.h"
#include "Token.h"
PGBN_INFIXEXPR_NAMESPACE_START

struct ExprNode {
    using NodeType = TokenType;
    struct Type {
        static constexpr Enum INVALID_NODE = 0;
        static constexpr Enum FUNCCALL_NODE = 1;
        static constexpr Enum VAL_NODE = 2;
    };

    std::uint32_t indexInPool{(std::uint32_t)-1};
    std::vector<ExprNode*> children;
    
    std::function<Value(ExprNode*, bool)> evalCallback = nullptr;
    std::function<OpCode(ExprNode*)> genOpCodeCallback = nullptr;
    std::function<void(ExprNode*)> pevalTask = nullptr; // only for parallel-eval

    NodeType nodeType = NodeType::INVALID;
    Enum type = 0;

    Value val;
    SymbolTable::Symbol * symbol = nullptr;
    std::promise<Value> valPromise; // only for parallel-eval
};

class NodePool {
    static constexpr std::uint32_t INIT_BUF_SIZE = 32;
    using Node = ExprNode;
public:
    NodePool();

    Node * get(std::uint32_t index);
    std::uint32_t count() const;

    Node * newNode();

    Node * newIfElseNode(Node * cond, Node * then, Node * els);
    Node * newBinOpNode(Node::NodeType op, Node * lNode, Node * rNode);
    Node * newUnaryOpNode(Node::NodeType op, Node * child);
    Node * newLiteralNode(Value val);
    Node * newBuiltinValNode(SymbolTable::Symbol * symbol);
    Node * newFunctionCallNode(SymbolTable::Symbol * symbol, std::vector<Node*> argNodes);

    static NodePool * getDefaultInstance();

private:
    Node m_initBuffer[INIT_BUF_SIZE];
    std::deque<Node> m_exBuffer;
    std::uint32_t m_count{0};
};

PGBN_INFIXEXPR_NAMESPACE_END
#endif // !PGBIGNUMBER_INFIXEXPR_EXPRTREE_H
