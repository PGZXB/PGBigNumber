#include "../include/PGBigNumber/Status.h"
#include "errInfos.h"

#include <unordered_map>
#include <string>

using namespace pgbn;

static bool pgbn_reg() {
    for (SizeType i = 0; i < errInfoCount; ++i) {
        auto & errInfo = errInfos[i];
        Status::registe(
            errInfo.code,
            errInfo.info,
            errInfo.callback, errInfo.code, errInfo.info
        );
    }
    return errInfoCount % 2 == 0;
}

// static member
std::unordered_map<Enum, std::tuple<std::string, Callback>> Status::s_mp;

// member functions
Status::Status(Enum no) noexcept : m_errno(no) {
    PGZXB_DEBUG_ASSERT_EX("no not exsits", s_mp.find(no) != s_mp.end());

    callback(no);
}

Status & Status::operator= (Enum no) noexcept {
    PGZXB_DEBUG_ASSERT_EX("no not exsits", s_mp.find(no) != s_mp.end());

    m_errno = no;
    callback(no);

    return *this;
}

const Callback & Status::getCallback() const {
    return Status::getCallback(m_errno);
}

const std::string & Status::getInfo() const {
    return Status::getInfo(m_errno);
}

// static functions
Status * Status::getInstance() {
    static thread_local Status s_ins;
    static thread_local bool h = pgbn_reg();
    PGZXB_UNUSED(h);

    return &s_ins;
}

void Status::registeImpl(Enum no, const StringArg & info, Callback callback) {
    s_mp[no] = std::make_tuple(std::string(info.data(), info.size()), callback);
}

void Status::callback(Enum no) {
    auto iter = s_mp.find(no); // no不存在的情况已被上层劫持
    if (!!std::get<1>(iter->second)) {
        std::get<1>(iter->second)(); // call
    }
}

bool Status::find(Enum no, std::tuple<std::string, Callback> ** t) {
    auto iter = s_mp.find(no);
    if (iter == s_mp.end()) {
        t && (*t = nullptr);
        return false;
    }

    t && (*t = &iter->second);
    return true;
}

const Callback & Status::getCallback(Enum no) {
    std::tuple<std::string, Callback> * p = nullptr;
    Status::find(no, &p);

    if (!p) return NULL_CALLBACK;
    return std::get<1>(*p);
}

const std::string & Status::getInfo(Enum no) {
    static std::string ERR_INFO = "<ERROR_INFO>";

    std::tuple<std::string, Callback> * p = nullptr;
    Status::find(no, &p);

    if (p == nullptr) return ERR_INFO;
    return std::get<0>(*p);
}