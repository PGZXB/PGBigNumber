#include <cstdint>
#include <cstdio>
#include <future>
#include <iostream>
#include <vector>

#include "parallel/ThreadPool.h"

using namespace pgbn::parallel;

int main() {
    setbuf(stdout, nullptr);

    auto genFunc = [](std::uint64_t i) {
        return [i]() {
            return i;
        };
    };

    ThreadPool pool(10);
    std::vector<std::future<std::uint64_t>> res;
    const std::uint64_t N = 100000;
    for (std::uint64_t i = 1; i <= N; ++i) {
        res.push_back(pool.addTask(genFunc(i)));
    }
    
    std::uint64_t sum = 0;
    for (auto &e : res) {
        sum += e.get();
    }

    PGZXB_DEBUG_ASSERT(sum == (N * (N + 1)) / 2);
    PGZXB_DEBUG_Print("PASSED");
}

