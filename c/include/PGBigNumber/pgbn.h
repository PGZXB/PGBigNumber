#ifndef PGBIGNUMBER_C_PGBN_H
#define PGBIGNUMBER_C_PGBN_H

/**
 * 
 * 提供C接口
 * 
*/

#include <inttypes.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef uint32_t pgbn_Enum_t;
typedef void* pgbn_CallbackArg_t;
typedef void (*pgbn_Callback_t) (pgbn_CallbackArg_t);

typedef struct pgbn_Status_t pgbn_Status_t;
typedef struct pgbn_BigInteger_t pgbn_BigInteger_t;

// pgbn::Status
pgbn_Status_t * pgbn_GetGlobalStatus();
void pgbn_RegisterStatus(pgbn_Enum_t no, const char * info);
void pgbn_RegisterStatusWithCallback(pgbn_Enum_t no, const char * info, pgbn_Callback_t callback, pgbn_CallbackArg_t arg);

pgbn_Status_t * pgbn_CreateStatus();
pgbn_Status_t * pgbn_CreateStatusWithErrno(pgbn_Enum_t no);
void pgbn_DestroyStatus(pgbn_Status_t * status);
void pgbn_SetErrnoForStatus(pgbn_Status_t * status, pgbn_Enum_t no);
const char * pgbn_GetInfoOfStatus(pgbn_Status_t * status);
const char * pgbn_GetInfoOfStatusWithErrno(pgbn_Enum_t no);

// pgbn::BigInteger(Impl)


#ifdef __cplusplus
} // end of extern "C"
#endif

#endif