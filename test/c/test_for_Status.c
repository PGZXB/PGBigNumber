#include <stdlib.h>
#include <stdio.h>

#include "../../c/include/PGBigNumber/pgbn.h"

int main () {
    pgbn_Status_t * globalStatus = pgbn_GetGlobalStatus();

    for (int i = 2; i <= 12; ++i) {
        pgbn_SetErrnoForStatus(globalStatus, i);
    }

    return 0;
}