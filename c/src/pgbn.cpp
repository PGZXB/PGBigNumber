#include "../include/PGBigNumber/pgbn.h"
#include "../../include/PGBigNumber/Status.h"

struct pgbn_Status_t {
    pgbn::Status * status;
};

struct pgbn_BigInteger_t {

};

// pgbn::Status
pgbn_Status_t * pgbn_GetGlobalStatus() {
    static pgbn_Status_t ins;
    ins.status = pgbn::Status::getInstance();
    return &ins; 
}

void pgbn_RegisterStatus(pgbn_Enum_t no, const char * info) {
    pgbn::Status::registe(no, info);
}

void pgbn_RegisterStatusWithCallback(
    pgbn_Enum_t no, const char * info, pgbn_Callback_t callback, pgbn_CallbackArg_t arg
) {
    pgbn::Status::registe(no, info, callback, arg);
}

pgbn_Status_t * pgbn_CreateStatus() {
    return pgbn_CreateStatusWithErrno(0);
}

pgbn_Status_t * pgbn_CreateStatusWithErrno(pgbn_Enum_t no) {
    pgbn_Status_t * result = new pgbn_Status_t;
    result->status = new pgbn::Status(no);
    return result;
}

void pgbn_DestroyStatus(pgbn_Status_t * status) {
    delete status->status;
    delete status;
}

void pgbn_SetErrnoForStatus(pgbn_Status_t * status, pgbn_Enum_t no) {
    status->status->operator=(no);
}

const char * pgbn_GetInfoOfStatus(pgbn_Status_t * status) {
    return status->status->getInfo().c_str();
}

const char * pgbn_GetInfoOfStatusWithErrno(pgbn_Enum_t no) {
    return pgbn::Status::getInfo(no).c_str();
}