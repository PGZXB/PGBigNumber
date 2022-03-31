#ifndef PGBIGNUMBER_PARALLEL_THREADPOOL_H
#define PGBIGNUMBER_PARALLEL_THREADPOOL_H

#include "fwd.h"
#include "SimpleConcurrenceQueue.h"

#include <atomic>
#include <memory>
#include <thread>
#include <future>

PGBN_PARALLEL_NAMESPACE_START
inline static auto kDefaultNThreads = std::thread::hardware_concurrency();

class ThreadPool {
    using TaskQueue = SimpleConcurrenceQueue<RawTask>;
public:
    ThreadPool(std::size_t nThreads = kDefaultNThreads) : m_running(true) {
        m_threads.resize(nThreads);
        for (auto &t : m_threads) {
            t = std::thread([this]() {
                while (true) {
                    RawTask task{nullptr};
                    if (m_tasks.pop(task)) {
                        PGZXB_DEBUG_ASSERT(!!task);
                        task();
                    }
                    if (!m_running && m_tasks.empty()) {
                        return;
                    }
                }
            });
        }
    }

    ~ThreadPool() {
        m_running = false;
        m_tasks.notifyAllPop();
        for (auto &t : m_threads) {
            PGZXB_DEBUG_ASSERT(t.joinable());
            t.join();
        }
    }

    template <typename F, typename ...Args>
    auto addTask(F &&func, Args &&...args) 
      -> std::future<typename std::result_of<F(Args...)>::type> {
        using ResultType = typename std::result_of<F(Args...)>::type;
        auto task = std::make_shared<std::packaged_task<ResultType()>>(
            std::bind(std::forward<F>(func), std::forward<Args>(args)...));

        auto res = task->get_future();

        std::atomic<bool> ok = true;
        m_tasks.push(
            [task]() { (*task)(); }, 
            [this]() { return !!this->m_running; },
            [&ok]() { ok = false; }
        );
        PGZXB_DEBUG_ASSERT(ok);
        return res;
    }

private:
    std::atomic<bool> m_running{false};
    TaskQueue m_tasks;
    std::vector<std::thread> m_threads;
};

PGBN_PARALLEL_NAMESPACE_END
#endif // !PGBIGNUMBER_PARALLEL_THREADPOOL_H
