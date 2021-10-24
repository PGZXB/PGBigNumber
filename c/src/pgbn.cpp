#include "../include/PGBigNumber/pgbn.h"
#include "../../include/PGBigNumber/Status.h"
#include "../../src/BigIntegerImpl.h"
#include "../../src/errInfos.h"

extern "C" {

struct pgbn_Status_t {
    pgbn::Status * status;
};

struct pgbn_String_t {
    std::string str;
};

struct pgbn_BigInteger_t {
    pgbn::BigIntegerImpl * impl;
};

// pgbn::Status
pgbn_Status_t * pgbn_Status_GetGlobalInstance() {
    static pgbn_Status_t ins;
    ins.status = pgbn::Status::getInstance();
    return &ins; 
}

void pgbn_Status_Register(pgbn_Enum_t no, const char * info) {
    pgbn::Status::registe(no, info);
}

void pgbn_Status_RegisterWithCallback(
    pgbn_Enum_t no, const char * info, pgbn_Callback_t callback, pgbn_CallbackArg_t arg
) {
    pgbn::Status::registe(no, info, callback, arg);
}

pgbn_Status_t * pgbn_Status_Create() {
    return pgbn_Status_CreateWithErrno(0);
}

pgbn_Status_t * pgbn_Status_CreateWithErrno(pgbn_Enum_t no) {
    pgbn_Status_t * result = new pgbn_Status_t;
    result->status = new pgbn::Status(no);
    return result;
}

void pgbn_Status_Destroy(pgbn_Status_t * status) {
    delete status->status;
    delete status;
}

void pgbn_Status_SetErrno(pgbn_Status_t * status, pgbn_Enum_t no) {
    status->status->operator=(no);
}

const char * pgbn_Status_GetInfo(pgbn_Status_t * status) {
    return status->status->getInfo().c_str();
}

const char * pgbn_Status_GetInfoByErrno(pgbn_Enum_t no) {
    return pgbn::Status::getInfo(no).c_str();
}

// pgbn_String_t
pgbn_String_t * pgbn_String_Create(const char * s) {
    pgbn_String_t * res = new pgbn_String_t;
    res->str = s;
    return res;
}

char * pgbn_String_CStr(pgbn_String_t * s) {
    s->str.c_str();
    return &s->str[0];
}

size_t pgbn_String_Length(pgbn_String_t * s) {
    return s->str.size();
}

void pgbn_String_Destroy(pgbn_String_t * s) {
    delete s;
}

// pgbn::BigInteger(Impl)
pgbn_BigInteger_t * pgbn_BigInteger_Create() {
    pgbn_BigInteger_t * res = new pgbn_BigInteger_t;
    res->impl = new pgbn::BigIntegerImpl;
    return res;
}

pgbn_BigInteger_t * pgbn_BigInteger_CreateWithCopying(const pgbn_BigInteger_t * other) {
    pgbn_BigInteger_t * res = new pgbn_BigInteger_t;
    res->impl = new pgbn::BigIntegerImpl(*other->impl);
    return res;
}

pgbn_BigInteger_t * pgbn_BigInteger_CreateWithI64(int64_t i64) {
    pgbn_BigInteger_t * res = new pgbn_BigInteger_t;
    res->impl = new pgbn::BigIntegerImpl(i64);
    return res;
}

pgbn_BigInteger_t * pgbn_BigInteger_CreateWithStr(const char * s, int radix) {
    pgbn_BigInteger_t * res = new pgbn_BigInteger_t;
    bool ok = true;
    res->impl = new pgbn::BigIntegerImpl;
    res->impl->fromString(s, radix, &ok);
    if (!ok) {
        delete res->impl;
        delete res;
        return NULL;
    }
    return res;
}

pgbn_BigInteger_t * pgbn_BigInteger_CreateWithBin(size_t bytes, const void * src, bool little) {
    pgbn_BigInteger_t * res = new pgbn_BigInteger_t;
    res->impl = new pgbn::BigIntegerImpl;
    res->impl->assign(src, bytes, little);
    return res;
}

void pgbn_BigInteger_Destroy(pgbn_BigInteger_t * b) {
    delete b->impl;
    delete b;
}

void pgbn_BigInteger_Copy(pgbn_BigInteger_t * dest, const pgbn_BigInteger_t * src) {
    dest->impl->assign(*src->impl);
}

void pgbn_BigInteger_AssignWithI64(pgbn_BigInteger_t * b, int64_t i64) {
    b->impl->assign(i64);
}

bool pgbn_BigInteger_AssignWithStr(pgbn_BigInteger_t * b, const char * s, int radix) {
    bool ok = true;
    b->impl->fromString(s, radix, &ok);
    return ok;
}

void pgbn_BigInteger_AssignWithBin(pgbn_BigInteger_t * b, const void * bin, size_t bytes, bool little) {
    b->impl->assign(bin, bytes, little);
}

int64_t pgbn_BigInteger_GetInt64(pgbn_BigInteger_t * b) {
    return pgbn_BigInteger_GetInt64WithChecking(b, NULL);
}

int64_t pgbn_BigInteger_GetInt64WithChecking(pgbn_BigInteger_t * b, bool * ok) {
    if (
        pgbn_BigInteger_CmpWithI64(b, std::numeric_limits<std::int64_t>::max()) > 0 ||
        pgbn_BigInteger_CmpWithI64(b, std::numeric_limits<std::int64_t>::min()) < 0
    ) {
        *pgbn::Status::getInstance() = pgbn::ErrCode::ARITHMETIC_OVERFLOW;
        ok && (*ok = false);
        return std::numeric_limits<std::int64_t>::max();
    }

    auto u64 = b->impl->toU64();
    *pgbn::Status::getInstance() = pgbn::ErrCode::SUCCESS;
    ok && (*ok = true);
    return static_cast<std::int64_t>(u64);
}

uint64_t pgbn_BigInteger_GetUInt64(pgbn_BigInteger_t * b) {
    return pgbn_BigInteger_GetUInt64WithChecking(b, NULL);
}

uint64_t pgbn_BigInteger_GetUInt64WithChecking(pgbn_BigInteger_t * b, bool * ok) {
    if (
        pgbn_BigInteger_CmpWithI64(b, std::numeric_limits<std::uint64_t>::max()) > 0 ||
        pgbn_BigInteger_CmpWithI64(b, std::numeric_limits<std::uint64_t>::min()) < 0
    ) {
        *pgbn::Status::getInstance() = pgbn::ErrCode::ARITHMETIC_OVERFLOW;
        ok && (*ok = false);
        return std::numeric_limits<std::uint64_t>::max();
    }

    auto u64 = b->impl->toU64();
    *pgbn::Status::getInstance() = pgbn::ErrCode::SUCCESS;
    ok && (*ok = true);
    return static_cast<std::int64_t>(u64);
}

uint32_t pgbn_BigInteger_Get2sComplementBlock(pgbn_BigInteger_t * b, size_t i) {
    return b->impl->getU32(i);
}

pgbn_String_t * pgbn_BigInteger_ToString(pgbn_BigInteger_t * b, int radix) {
    pgbn_String_t * str = pgbn_String_Create(b->impl->toString(radix).c_str());
    return str;
}

size_t pgbn_BigInteger_GetMagData(pgbn_BigInteger_t * b, void * dest, size_t maxlen) {
    return b->impl->copyMagDataTo(dest, maxlen);
}

bool pgbn_BigInteger_IsZero(pgbn_BigInteger_t * b) {
    return b->impl->isZero();
}

bool pgbn_BigInteger_IsOne(pgbn_BigInteger_t * b) {
    return b->impl->isOne();
}

bool pgbn_BigInteger_IsNegOne(pgbn_BigInteger_t * b) {
    return b->impl->isNegOne();
}

bool pgbn_BigInteger_IsPositive(pgbn_BigInteger_t * b) {
    return b->impl->flagsContains(pgbn::BNFlag::POSITIVE);
}

bool pgbn_BigInteger_IsNegative(pgbn_BigInteger_t * b) {
    return b->impl->flagsContains(pgbn::BNFlag::NEGATIVE);
}

bool pgbn_BigInteger_IsEven(pgbn_BigInteger_t * b) {
    return b->impl->isEven();
}

bool pgbn_BigInteger_IsOdd(pgbn_BigInteger_t * b) {
    return b->impl->isOdd();
}

void pgbn_BigInteger_Inc(pgbn_BigInteger_t * b) {
    b->impl->inc();
}

void pgbn_BigInteger_Dec(pgbn_BigInteger_t * b) {
    b->impl->dec();
}

void pgbn_BigInteger_NotSelf(pgbn_BigInteger_t * b) {
    b->impl->notSelf();
}

void pgbn_BigInteger_AddAssign(pgbn_BigInteger_t * a, pgbn_BigInteger_t * b){
    a->impl->addAssign(*b->impl);
}

void pgbn_BigInteger_AddAssignWithI64(pgbn_BigInteger_t * a, int64_t i64){
    a->impl->addAssign(i64);
}

void pgbn_BigInteger_SubAssign(pgbn_BigInteger_t * a, pgbn_BigInteger_t * b){
    a->impl->subAssign(*b->impl);
}

void pgbn_BigInteger_SubAssignWithI64(pgbn_BigInteger_t * a, int64_t i64){
    a->impl->subAssign(i64);
}

void pgbn_BigInteger_MulAssign(pgbn_BigInteger_t * a, pgbn_BigInteger_t * b){
    a->impl->mulAssign(*b->impl);
}

void pgbn_BigInteger_MulAssignWithI64(pgbn_BigInteger_t * a, int64_t i64){
    a->impl->mulAssign(i64);
}

void pgbn_BigInteger_DivAssign(pgbn_BigInteger_t * a, pgbn_BigInteger_t * b){
    a->impl->divAssign(*b->impl);
}

void pgbn_BigInteger_DivAssignWithI64(pgbn_BigInteger_t * a, int64_t i64){
    a->impl->divAssign(i64);
}

void pgbn_BigInteger_ModAssign(pgbn_BigInteger_t * a, pgbn_BigInteger_t * b){
    pgbn::BigIntegerImpl q;
    a->impl->modAssignAndQuotient(*b->impl, q);
}

void pgbn_BigInteger_ModAssignWithI64(pgbn_BigInteger_t * a, int64_t i64){
    pgbn::BigIntegerImpl q;
    a->impl->modAssignAndQuotient(i64, q);
}

void pgbn_BigInteger_AndAssign(pgbn_BigInteger_t * a, pgbn_BigInteger_t * b){
    a->impl->andAssign(*b->impl);
}

void pgbn_BigInteger_AndAssignWithI64(pgbn_BigInteger_t * a, int64_t i64){
    a->impl->andAssign(i64);
}

void pgbn_BigInteger_OrAssign(pgbn_BigInteger_t * a, pgbn_BigInteger_t * b){
    a->impl->orAssign(*b->impl);
}

void pgbn_BigInteger_OrAssignWithI64(pgbn_BigInteger_t * a, int64_t i64){
    a->impl->orAssign(i64);
}

void pgbn_BigInteger_XorAssign(pgbn_BigInteger_t * a, pgbn_BigInteger_t * b){
    a->impl->xorAssign(*b->impl);
}

void pgbn_BigInteger_XorAssignWithI64(pgbn_BigInteger_t * a, int64_t i64){
    a->impl->xorAssign(i64);
}

void pgbn_BigInteger_SHLAssignWithI64(pgbn_BigInteger_t * a, int64_t i64){
    a->impl->shiftLeftAssign(i64);
}

void pgbn_BigInteger_SHRAssignWithI64(pgbn_BigInteger_t * a, int64_t i64){
    a->impl->shiftRightAssign(i64);
}

void pgbn_BigInteger_Negate(pgbn_BigInteger_t * b) {
    b->impl->negate();
}

void pgbn_BigInteger_AbsSelf(pgbn_BigInteger_t * b) {
    b->impl->abs();
}

void pgbn_BigInteger_NotSelfBits(pgbn_BigInteger_t * b) {
    b->impl->notBits();
}

int pgbn_BigInteger_CmpWithI64(pgbn_BigInteger_t * a, int64_t i64) {
    return a->impl->cmp(pgbn::BigIntegerImpl(i64));
}

int pgbn_BigInteger_Cmp(pgbn_BigInteger_t * a, pgbn_BigInteger_t * b) {
    return a->impl->cmp(*b->impl);
}

} // end of extern C