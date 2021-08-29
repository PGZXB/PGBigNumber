#include <iostream>
#include <random>
#include <chrono>
#include <ctime>
#include <cassert>
#include "../src/BigIntegerImpl.h"

// Simple Test For fromString And toString Of BigIntegerImpl

using namespace pgbn;

int main () {
    char chars[] = {
    //        2    3    4    5    6    7    8    9    10  进制
        '0', '1', '2', '3', '4', '5', '6', '7', '8', '9',
    //  11   12   13   14   15   16   17   18   19   20   进制
        'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J',
    // ...
        'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T',
    // ...                   35   36                      进制
        'U', 'V', 'W', 'X', 'Y', 'Z'
    };

    char sig[] = { '+', '-' };

    constexpr int N = 100;
    constexpr int MAX_BYTES = 64;

    std::default_random_engine eng(std::time(nullptr));
    std::uniform_int_distribution<int> gLen(1, MAX_BYTES);
    std::uniform_int_distribution<int> gPosiOrNeg(0, 1);

    std::vector<BigIntegerImpl> nums;
    nums.reserve(128);

    std::string s;
    s.reserve(MAX_BYTES);
    for (int j = 2; j <= 36; ++j) {
        std::uniform_int_distribution<int> gRange(0, j - 1);
        for (int i = 0; i < N; ++i) {
            s.clear();
            s.push_back(sig[gPosiOrNeg(eng)]);
            auto len = gLen(eng);
            for (int k = 0; k < len; ++k) {
                s.push_back(chars[gRange(eng)]);
            }
            // std::cout << s << "\n";
            nums.emplace_back().fromString(s, j);
        }
        // std::cout << "\n";
    }

    std::size_t mcroSec = 0.f;

    for (const auto & n : nums) {
        for (int i = 2; i <= 36; ++i) {
            std::size_t start = std::chrono::steady_clock::now().time_since_epoch().count();
            std::string s = n.toString(i);
            BigIntegerImpl temp(s, i);
            mcroSec += (std::chrono::steady_clock::now().time_since_epoch().count() - start) / 1000;
            assert(s == temp.toString(i));
        }
    }

    std::cout << "Run fromString And toString in " << mcroSec << "μs. All Examples Passed!!\n";

    return 0;
}