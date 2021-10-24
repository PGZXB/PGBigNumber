#ifndef PGBIGNUMBER_C_PGBN_H
#define PGBIGNUMBER_C_PGBN_H

/**
 * 
 * 提供C接口
 * 
*/

#include <inttypes.h>
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef uint32_t pgbn_Enum_t;
typedef void* pgbn_CallbackArg_t;
typedef void (*pgbn_Callback_t) (pgbn_CallbackArg_t);

typedef struct pgbn_Status_t pgbn_Status_t;
typedef struct pgbn_String_t pgbn_String_t;
typedef struct pgbn_BigInteger_t pgbn_BigInteger_t;

// pgbn::Status
pgbn_Status_t * pgbn_Status_GetGlobalInstance();
void pgbn_Status_Register(pgbn_Enum_t no, const char * info);
void pgbn_Status_RegisterWithCallback(pgbn_Enum_t no, const char * info, pgbn_Callback_t callback, pgbn_CallbackArg_t arg);
pgbn_Status_t * pgbn_Status_Create();
pgbn_Status_t * pgbn_Status_CreateWithErrno(pgbn_Enum_t no);
void pgbn_Status_Destroy(pgbn_Status_t * status);
void pgbn_Status_SetErrno(pgbn_Status_t * status, pgbn_Enum_t no);
const char * pgbn_Status_GetInfo(pgbn_Status_t * status);
const char * pgbn_Status_GetInfoByErrno(pgbn_Enum_t no);

// pgbn_String_t
pgbn_String_t * pgbn_String_Create(const char *);
char * pgbn_String_CStr(pgbn_String_t * s);
size_t pgbn_String_Length(pgbn_String_t * s);
void pgbn_String_Destroy(pgbn_String_t * s);

// pgbn::BigInteger(Impl)
pgbn_BigInteger_t * pgbn_BigInteger_Create();
pgbn_BigInteger_t * pgbn_BigInteger_CreateWithCopying(const pgbn_BigInteger_t * other);
pgbn_BigInteger_t * pgbn_BigInteger_CreateWithI64(int64_t i64);
pgbn_BigInteger_t * pgbn_BigInteger_CreateWithStr(const char * s, int radix);
pgbn_BigInteger_t * pgbn_BigInteger_CreateWithBin(size_t bytes, const void * src, bool little);
void pgbn_BigInteger_Destroy(pgbn_BigInteger_t * b);
void pgbn_BigInteger_Copy(pgbn_BigInteger_t * dest, const pgbn_BigInteger_t * src);
void pgbn_BigInteger_AssignWithI64(pgbn_BigInteger_t * b, int64_t i64);
bool pgbn_BigInteger_AssignWithStr(pgbn_BigInteger_t * b, const char * s, int radix);
void pgbn_BigInteger_AssignWithBin(pgbn_BigInteger_t * b, const void * bin, size_t bytes, bool little);
int64_t pgbn_BigInteger_GetInt64(pgbn_BigInteger_t * b);
int64_t pgbn_BigInteger_GetInt64WithChecking(pgbn_BigInteger_t * b, bool * ok);
uint64_t pgbn_BigInteger_GetUInt64(pgbn_BigInteger_t * b);
uint64_t pgbn_BigInteger_GetUInt64WithChecking(pgbn_BigInteger_t * b, bool * ok);
uint32_t pgbn_BigInteger_Get2sComplementBlock(pgbn_BigInteger_t * b, size_t i);
pgbn_String_t * pgbn_BigInteger_ToString(pgbn_BigInteger_t * b, int radix);
size_t pgbn_BigInteger_GetMagData(pgbn_BigInteger_t * b, void * dest, size_t maxlen);
bool pgbn_BigInteger_IsZero(pgbn_BigInteger_t * b);
bool pgbn_BigInteger_IsOne(pgbn_BigInteger_t * b);
bool pgbn_BigInteger_IsNegOne(pgbn_BigInteger_t * b);
bool pgbn_BigInteger_IsPositive(pgbn_BigInteger_t * b);
bool pgbn_BigInteger_IsNegative(pgbn_BigInteger_t * b);
bool pgbn_BigInteger_IsEven(pgbn_BigInteger_t * b);
bool pgbn_BigInteger_IsOdd(pgbn_BigInteger_t * b);
void pgbn_BigInteger_Inc(pgbn_BigInteger_t * b);
void pgbn_BigInteger_Dec(pgbn_BigInteger_t * b);
void pgbn_BigInteger_NotSelf(pgbn_BigInteger_t * b);
void pgbn_BigInteger_AddAssign(pgbn_BigInteger_t * a, pgbn_BigInteger_t * b);
void pgbn_BigInteger_AddAssignWithI64(pgbn_BigInteger_t * a, int64_t i64);
void pgbn_BigInteger_SubAssign(pgbn_BigInteger_t * a, pgbn_BigInteger_t * b);
void pgbn_BigInteger_SubAssignWithI64(pgbn_BigInteger_t * a, int64_t i64);
void pgbn_BigInteger_MulAssign(pgbn_BigInteger_t * a, pgbn_BigInteger_t * b);
void pgbn_BigInteger_MulAssignWithI64(pgbn_BigInteger_t * a, int64_t i64);
void pgbn_BigInteger_DivAssign(pgbn_BigInteger_t * a, pgbn_BigInteger_t * b);
void pgbn_BigInteger_DivAssignWithI64(pgbn_BigInteger_t * a, int64_t i64);
void pgbn_BigInteger_ModAssign(pgbn_BigInteger_t * a, pgbn_BigInteger_t * b);
void pgbn_BigInteger_ModAssignWithI64(pgbn_BigInteger_t * a, int64_t i64);
void pgbn_BigInteger_AndAssign(pgbn_BigInteger_t * a, pgbn_BigInteger_t * b);
void pgbn_BigInteger_AndAssignWithI64(pgbn_BigInteger_t * a, int64_t i64);
void pgbn_BigInteger_OrAssign(pgbn_BigInteger_t * a, pgbn_BigInteger_t * b);
void pgbn_BigInteger_OrAssignWithI64(pgbn_BigInteger_t * a, int64_t i64);
void pgbn_BigInteger_XorAssign(pgbn_BigInteger_t * a, pgbn_BigInteger_t * b);
void pgbn_BigInteger_XorAssignWithI64(pgbn_BigInteger_t * a, int64_t i64);
void pgbn_BigInteger_SHLAssignWithI64(pgbn_BigInteger_t * a, int64_t i64);
void pgbn_BigInteger_SHRAssignWithI64(pgbn_BigInteger_t * a, int64_t i64);
void pgbn_BigInteger_Negate(pgbn_BigInteger_t * b);
void pgbn_BigInteger_AbsSelf(pgbn_BigInteger_t * b);
void pgbn_BigInteger_NotSelfBits(pgbn_BigInteger_t * b);
int pgbn_BigInteger_CmpWithI64(pgbn_BigInteger_t * a, int64_t);
int pgbn_BigInteger_Cmp(pgbn_BigInteger_t * a, pgbn_BigInteger_t * b);

#ifdef __cplusplus
} // end of extern "C"
#endif

#endif