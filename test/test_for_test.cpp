#include <PGBigNumber/pgdebug.h>

int test () {
    return __cplusplus;
}

int main () {
    const char * hello_msg = "Hello PGBigNumber!!";

    PGZXB_DEBUG_PrintVar(hello_msg);
    PGZXB_DEBUG_CallFunc(test());

    return 0;
}