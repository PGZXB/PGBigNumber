#include "Status.h"

#include <iostream>

#define STATUS (*pgbn::Status::getInstance())

std::int8_t a = 0, b = 0;
std::int8_t c = 0;

enum class CALCU_STATUS : pgbn::Enum {
    SUCCESS,
    DIV_ZERO_ERR,
    ADD_OVERFLOW_ERR,
    SUB_OVERFLOW_ERR,
    ADD_GLOB_A_AND_B_ERROR,
};

#define TO_ENUM(val) static_cast<pgbn::Enum>(val)

std::int8_t add(std::int8_t a, std::int8_t b) {
    std::int8_t res = a + b;
    if (
        (a > 0 && b > 0 && res <= 0) ||
        (a < 0 && b < 0 && res >= 0)
    ) STATUS = TO_ENUM(CALCU_STATUS::ADD_OVERFLOW_ERR);
    else {
        STATUS = TO_ENUM(CALCU_STATUS::SUCCESS);
    }

    return res;
}

void add(std::int8_t a, std::int8_t b, std::int8_t & res) {
    res = a + b;
    if (
        (a > 0 && b > 0 && res <= 0) ||
        (a < 0 && b < 0 && res >= 0)
    ) STATUS = TO_ENUM(CALCU_STATUS::ADD_GLOB_A_AND_B_ERROR);
    else {
        STATUS = TO_ENUM(CALCU_STATUS::SUCCESS);
    }
}

std::int8_t sub(std::int8_t a, std::int8_t b) {
    std::int8_t res = a - b;
    if (
        (a > 0 && b < 0 && res <= 0) ||
        (a < 0 && b > 0 && res >= 0)
    ) STATUS = TO_ENUM(CALCU_STATUS::SUB_OVERFLOW_ERR);
    else {
        STATUS = TO_ENUM(CALCU_STATUS::SUCCESS);
    }

    return res;
}

std::int8_t divide(std::int8_t a, std::int8_t b) {

    if (b == 0) STATUS = TO_ENUM(CALCU_STATUS::DIV_ZERO_ERR);
    else STATUS = TO_ENUM(CALCU_STATUS::SUCCESS);

    return a / b;
}

void err_output(pgbn::Enum err_no) {
    std::cerr << err_no << " : " << pgbn::Status::getInfo(err_no) << '\n';
}

void add_err(std::int8_t & a, std::int8_t & b, std::int8_t & c) {
    std::cerr << "Calcu " << (int)a << " + " << (int)b << " = " << (int)c << " Error\n";
    err_output(
        static_cast<pgbn::Enum>(CALCU_STATUS::ADD_GLOB_A_AND_B_ERROR)
    );
}

void err_exit(pgbn::Enum err_no) {
    err_output(err_no);
    std::cerr << "Exiting---\n";
    exit(-1);
}

void test() {
    std::cerr << "TEST\n";
}

#define REG pgbn::Status::registe

int main () {

    // // 注册错误码
    REG(CALCU_STATUS::SUCCESS, "SUCCESS");
    REG(CALCU_STATUS::ADD_OVERFLOW_ERR, "Add Overflow\n", err_output, TO_ENUM(CALCU_STATUS::ADD_OVERFLOW_ERR));
    REG(CALCU_STATUS::SUB_OVERFLOW_ERR, "Sub Overflow\n", err_output, TO_ENUM(CALCU_STATUS::SUB_OVERFLOW_ERR));
    REG(CALCU_STATUS::ADD_GLOB_A_AND_B_ERROR, "Add a And b Error\n", add_err, std::ref<int8_t>(a), std::ref<int8_t>(b), std::ref<int8_t>(c));
    REG(CALCU_STATUS::DIV_ZERO_ERR, "DIV 0 Error", err_exit, TO_ENUM(CALCU_STATUS::DIV_ZERO_ERR));

    add(1, 2); // success

    add(100, 100); // overflow

    sub(2, 4); // success

    sub(100, -100); // overflow

    a = b = 100;
    add(a, b, c); // error

    divide(1, 0);

    return 0;
}
