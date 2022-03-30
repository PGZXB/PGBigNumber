#include <iostream>
#include "../include/PGBigNumber/pgdebug.h"
#include "../src/Slice.h"

int main () {

    std::shared_ptr<std::vector<int>> pData(new std::vector<int>{1, 2, 3, 4, 5});
    pgbn::Slice<int> s(pData);

    std::cout << pgfmt::format("0 : {0}\n", s);

    s.prepend(-1).prepend(-2);
    std::cout << pgfmt::format("1 : {0}\n", s);

    for (int i = 0; i < 15; ++i) {
        s.prepend(i);
    }
    std::cout << pgfmt::format("2 : {0}\n", s);

    for (int i = 0; i < 10; ++i) {
        s.append(i);
    }
    std::cout << pgfmt::format("3 : {0}\n", s);

    std::cout << "4 : [";
    for (int i = 0; i < static_cast<int>(s.count()); ++i) {
        std::cout << s[i] << ", ";
    }
    std::cout << "]\n";

    std::cout << pgfmt::format("5 : {0}\n", s.slice(15, 22));
    s = s.sliced(0, 4);
    std::cout << pgfmt::format("6 : {0}\n", s);

    pgbn::Slice<int> copy = s;
    std::cout << pgfmt::format("hasSameBaseData : {0}\n", copy.hasSameBaseData(s));

    std::cout << pgfmt::format("7 : {0}\n", copy);

    copy.cloneData();

    std::cout << pgfmt::format("hasSameBaseData : {0}\n", copy.hasSameBaseData(s));
    copy[0] = 100;
    copy[1] = 1000;
    std::cout << pgfmt::format("8 : {0}\n", copy);
    std::cout << pgfmt::format("9 : {0}\n", s);

    std::cout << pgfmt::format("10 : {0}\n", copy.slice(0, 7));

    return 0;
}
