#include <stdio.h>
#include <string.h>
#include <assert.h>

#include "PGBigNumber/pgbn.h"

int main() {

    pgbn_BigInteger_t *a = pgbn_BigInteger_CreateWithStr("1000000000000000000000000000000000000", 10);
    pgbn_BigInteger_t *b = pgbn_BigInteger_CreateWithStr("2", 10);
    pgbn_BigInteger_t *c = pgbn_BigInteger_CreateWithStr("-1000000000000000000000000000000000000", 10);

    pgbn_String_t *a_str = pgbn_BigInteger_ToString(a, 10);
    pgbn_String_t *b_str = pgbn_BigInteger_ToString(b, 10);
    pgbn_String_t *c_str = pgbn_BigInteger_ToString(c, 10);

    // d = a * b
    pgbn_BigInteger_t *d = pgbn_BigInteger_CreateWithCopying(a);
    pgbn_BigInteger_MulAssign(d, b);
    
    pgbn_String_t *d_str = pgbn_BigInteger_ToString(d, 10);
    assert(strcmp((pgbn_String_CStr(d_str)), "2000000000000000000000000000000000000") == 0);
    pgbn_String_Destroy(d_str);
    
    // d /= c
    pgbn_BigInteger_DivAssign(d, c);
    
    d_str = pgbn_BigInteger_ToString(d, 10);
    assert(strcmp((pgbn_String_CStr(d_str)), "-2") == 0);

    printf("%s * %s / %s == %s",
        pgbn_String_CStr(a_str),
        pgbn_String_CStr(b_str),
        pgbn_String_CStr(c_str),
        pgbn_String_CStr(d_str)
    );

    // Destroy
    pgbn_BigInteger_Destroy(a);
    pgbn_BigInteger_Destroy(b);
    pgbn_BigInteger_Destroy(c);
    pgbn_BigInteger_Destroy(d);

    pgbn_String_Destroy(a_str);
    pgbn_String_Destroy(b_str);
    pgbn_String_Destroy(c_str);
    pgbn_String_Destroy(d_str);
    return 0;
}
