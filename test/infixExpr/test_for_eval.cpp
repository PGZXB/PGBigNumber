#include <iostream>

#include "../../src/infixExpr/ExprParser.h"
#include "../../src/infixExpr/Token.h"
#include "../../src/infixExpr/ExprTree.h"
#include "../../include/PGBigNumber/streams/StringReadStream.h"

using namespace pgbn::infixExpr;

void printTree(ExprNode * node) {
    if (!node) { std::cout << "NULL, "; return; }
    std::cout << "[" << int(node->nodeType) << " : " << node->val.toString(10) << "], ";
    for (auto * ch : node->children) {
        printTree(ch);
    }
}

int main () {
    
    auto pST = SymbolTable::getInstance();
    pST->registe("sum", SymbolTable::Symbol{Func{
        .nativeCall = [] (std::vector<Value> args) -> Value {
            Value res;
            for (auto & e : args) {
                res.addAssign(e);
            }
            return res;
        },
        .argCount = -1
    }});
    pST->registe("min", SymbolTable::Symbol{Func{.argCount = -1}});
    pST->registe("_100PI", SymbolTable::Symbol{Value{314}});

    const char testSrc1[] = 
        "sum(1, sum(2, sum(4, sum(5)))) / _100PI";

    pg::base::StringReadStream testSrc1Stream(testSrc1);
    ExprParser parser(testSrc1Stream, *Hooks::getInstance(), NodePool::getDefaultInstance());
    
    auto * root = parser.parse();
    printTree(root); std::cout << "\n";

    std::cout << root->evalCallback(root).toString(10) << "\n";

    return 0;
}
