#include "../src/BigIntegerImpl.h"
#include <algorithm>
#include <iostream>
#include <bitset>

using namespace pgbn;

int main () {
    BigIntegerImpl a, b;

    // a.assign(100);
    // b.assign(100);
    // PGZXB_DEBUG_ASSERT(a.cmp(b) == 0);

    // a.assign(100);
    // b.assign(a);
    // PGZXB_DEBUG_ASSERT(a.cmp(b) == 0);

    // a.assign(0);
    // b.assign(0);
    // PGZXB_DEBUG_ASSERT(a.cmp(b) == 0);

    // a.assign(100000);
    // b.assign(-10);
    // PGZXB_DEBUG_ASSERT(a.cmp(b) == 1);

    // a.assign(-110);
    // b.assign(10);
    // PGZXB_DEBUG_ASSERT(a.cmp(b) == -1);
    
    // int len = 5 * 32 + 10;
    int len = 164;
    Byte * bigNum = (Byte *)std::malloc(len);
    for (int i = 0; i < len; ++i) {
        bigNum[i] = std::rand() % 255;
        if (i == 0 || i == len - 1) std::cout << std::bitset<8>(0);
        else std::cout << std::bitset<8>(bigNum[i]);
    }
    std::cout << "\n-----------------------\n";
    // bigNum[0] = +1;
    // a.assign(bigNum, len, false);
    // b.assign(100);
    // PGZXB_DEBUG_ASSERT(a.cmp(b) == +1);

    // std::reverse(bigNum, bigNum + len);
    // b.assign(bigNum, len, true);
    // PGZXB_DEBUG_ASSERT(b.cmp(a) == 0);
    bigNum[0] = bigNum[len - 1] = 0x0;
    a.assign(bigNum, len);
    b.assign(bigNum, len, false);

    // a.assign(100);
    // b.assign(100);

    std::cout << a.toString(16) << '\n';
    std::cout << "\n-----------------------\n";

    std::cout << b.toString(16) << '\n';
    std::cout << "\n-----------------------\n";

    a.assign(0);
    a.subAssign(b);
    std::cout << a.toString(16) << '\n';
    std::cout << "\n-----------------------\n";

    // a.subAssign(a);
    // std::cout << a.toString(16) << '\n';

    free(bigNum);
    return 0;
}