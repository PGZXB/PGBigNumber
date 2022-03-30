#include <iostream>
#include "BigIntegerImpl.h"

// Simple Test
// 更具体的测试利用test_for_calcu.cpp

using namespace pgbn;

int main () {

    BigIntegerImpl a;
    char s[] = "+1381274902174901ABCDFF969691362ABDC0027409163026301";
    a.fromString(s, 16);
    std::cout << a.toString(16) << "\n";

    return 0;
}
