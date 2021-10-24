#include <stdlib.h>
#include <stdio.h>

#include "../../c/include/PGBigNumber/pgbn.h"

typedef struct Info {
    const char * msg;
    pgbn_Enum_t no;
} Info;

void printNoAndInfo(void * arg) {
    const Info * info = (const Info *)arg;
    printf("[%" PRIu32 "]%s\n", info->no, info->msg);
}

int main () {
    pgbn_Status_t * globalStatus = pgbn_Status_GetGlobalInstance();

    for (int i = 2; i <= 12; ++i) {
        pgbn_Status_SetErrno(globalStatus, i);
    }

    Info info = { .msg = "Test(Code : 100)", .no = 100 };
    pgbn_Status_RegisterWithCallback(100, "Test(Code : 100)", printNoAndInfo, &info);
    pgbn_Status_SetErrno(globalStatus, 100);

    info.msg = "Test For Status#100";

    pgbn_Status_t * s = pgbn_Status_Create();
    pgbn_Status_SetErrno(s, 100);
    pgbn_Status_Destroy(s);

    return 0;
}