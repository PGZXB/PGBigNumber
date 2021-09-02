#ifndef PGBIGNUMBER_STATUS_H
#define PGBIGNUMBER_STATUS_H

#include "fwd.h"
PGBN_NAMESPACE_START
// 用来存放执行状态(错误码)的类
// 允许注册错误码和提示信息
// 允许指定错误回调
// 提供getInstence获取thread_local单例
// 只允许move, 不允许拷贝

class Status {
public:
    Status() = default;
    Status(Enum no) noexcept; // 设置m_errno并回调

    Status(const Status &) = delete;
    Status(Status &&) = default;

    Status & operator= (const Status &) = delete;
    Status & operator= (Status &&) = default;
    Status & operator= (Enum no) noexcept;

    const Callback & getCallback() const;
    const std::string & getInfo() const;
public:
    static constexpr Enum INVALID = static_cast<Enum>(-1);

    static Status * getInstance();

    template<typename E>
    static void registe(E no, const StringArg & info) {
        Status::registeImpl(static_cast<Enum>(no), info, NULL_CALLBACK);
    }

    template<typename E, typename FUNC, typename ... ARGS> // 线程不安全; 覆盖;
    static void registe(E no, const StringArg & info, FUNC && callback, ARGS && ... args) {
        Status::registeImpl(static_cast<Enum>(no), info, std::bind(std::forward<FUNC>(callback), std::forward<ARGS>(args)...));
    }

    static bool find(Enum no, std::tuple<std::string, Callback> ** t = nullptr); // 存在返回true
    static const Callback & getCallback(Enum no); // 获取回调, 如果no不存在则返回空
    static const std::string & getInfo(Enum no);  // 获取信息, 不在在返回"<ERROR_INFO>"
private:
    static std::unordered_map<Enum, std::tuple<std::string, Callback>> s_mp;

    static void registeImpl(Enum no, const StringArg & info, Callback callback);

    static void callback(Enum no);
private:
    Enum m_errno = INVALID;
};

PGBN_NAMESPACE_END

#define GetGlobalStatus() (*(Status::getInstance()))

#endif