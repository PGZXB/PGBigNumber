#ifndef PGBIGNUMBER_PARALLEL_SCQUEUE_H
#define PGBIGNUMBER_PARALLEL_SCQUEUE_H

#include "fwd.h"

#include <atomic>
#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>

PGBN_PARALLEL_NAMESPACE_START

template<typename DataType, typename Sequence = std::deque<DataType>>
class SimpleConcurrenceQueue {
public:
    SimpleConcurrenceQueue() = default;

    SimpleConcurrenceQueue(const SimpleConcurrenceQueue & other) {
        std::lock_guard<std::mutex> _(other.m_mutex);
        m_data = other.m_data;
    }

    SimpleConcurrenceQueue & operator= (const SimpleConcurrenceQueue &) = delete;
    SimpleConcurrenceQueue(SimpleConcurrenceQueue &&) = delete;
    SimpleConcurrenceQueue & operator= (SimpleConcurrenceQueue &&) = delete;
    
    ~SimpleConcurrenceQueue() = default;

    bool empty() const {
        std::lock_guard<std::mutex> _(m_mutex);
        return m_data.empty();
    }
    
    template <typename Condition, typename ErrCallback>
    void push(const DataType &data, Condition cond, ErrCallback err) {
        std::lock_guard<std::mutex> _(m_mutex);
        if (!cond()) {
            err();
            return;
        }
        m_data.emplace(data);
        m_cond.notify_one();
    }
    
    template <typename Condition, typename ErrCallback>
    void push(DataType &&data, Condition cond, ErrCallback err) {
        std::lock_guard<std::mutex> _(m_mutex);
        if (!cond()) {
            err();
            return;
        }
        m_data.emplace(std::move(data));
        m_cond.notify_one();
    }
    
    bool tryPop(DataType &val) {
        std::lock_guard<std::mutex> _(m_mutex);
        if (m_data.empty()) return false;
        val = std::move(m_data.front());
        m_data.pop();
        return true;
    }
    
    bool pop(DataType &val) {
        std::unique_lock<std::mutex> lg(m_mutex);
        m_cond.wait(lg, [this] { return m_alwaysPop || !m_data.empty(); });
        if (m_data.empty()) return false;
        val = std::move(m_data.front());
        m_data.pop();
        return true;
    }

    void notifyAllPop() {
        m_alwaysPop = true;
        m_cond.notify_all();
    }
    
private:
    std::queue<DataType, Sequence> m_data;
    mutable std::mutex m_mutex;
    std::condition_variable m_cond;
    bool m_alwaysPop{false};
};

PGBN_PARALLEL_NAMESPACE_END
#endif
