#ifndef PGBIGNUMBER_SLICE_H
#define PGBIGNUMBER_SLICE_H

#include <vector>
#include <memory>
#include <algorithm>
#include <limits>
#include "../include/PGBigNumber/fwd.h"
PGBN_NAMESPACE_START

// manage the range of vector
template <typename T = int, typename A = std::allocator<T>>
class Slice {
    static constexpr SizeType DEFAULT_PRE_SPACE = 1;
    using Vec = std::vector<T, A>;
public:
    using iterator = typename Vec::iterator;
    using const_iterator = typename Vec::const_iterator;

    Slice() : m_data(new Vec(DEFAULT_PRE_SPACE * 2)),
              m_lo(DEFAULT_PRE_SPACE), m_hi(DEFAULT_PRE_SPACE) {

    }
    
    explicit Slice(
        std::shared_ptr<Vec> pData,
        SizeType lo = 0, SizeType hi = std::numeric_limits<SizeType>::max()
    ) : m_data(pData) {
        PGZXB_DEBUG_ASSERT_EX("pData must not be nullptr", !!pData);
        PGZXB_DEBUG_ASSERT_EX("lo must be less than hi", lo < hi);

        if (hi == std::numeric_limits<SizeType>::max()) hi = pData->size();

        const SizeType len = pData->size();
        if (hi > len) pData->resize(hi);

        m_lo = lo;
        m_hi = hi;
    }

    Slice(const Slice &) = default;

    Slice(Slice && other) noexcept
        : m_data(std::move(other.m_data)),
          m_lo(other.m_lo),
          m_hi(other.m_hi) {

        other.m_data.reset(new Vec(DEFAULT_PRE_SPACE * 2));
        other.m_lo = DEFAULT_PRE_SPACE;
        other.m_hi = DEFAULT_PRE_SPACE;
    }

    Slice & operator= (const Slice &) = default;

    Slice & operator= (Slice && other) {

        m_data = std::move(other.m_data);
        m_lo = other.m_lo;
        m_hi = other.m_hi;

        other.m_data.reset(new Vec(DEFAULT_PRE_SPACE * 2));
        other.m_lo = DEFAULT_PRE_SPACE;
        other.m_hi = DEFAULT_PRE_SPACE;

        return *this;
    }

    Slice & cloneData() {
        if (!m_data || m_data.use_count() == 1) return *this;

        std::shared_ptr<Vec> newData(new Vec(DEFAULT_PRE_SPACE + m_hi - m_lo));
        std::copy(m_data->begin() + m_lo, m_data->begin() + m_hi, newData->begin() + DEFAULT_PRE_SPACE);
        
        m_data = newData;
        m_lo = DEFAULT_PRE_SPACE;
        m_hi = newData->size();

        return *this;
    }

    Slice & prepend(const T & data) {
        if (m_lo > 0) {
            (*m_data)[--m_lo] = data;
        } else {
            m_data->insert(m_data->begin(), data);
            ++m_hi;
        }

        return *this;
    }

    Slice & append(const T & data) {
        if (m_hi < m_data->size()) {
            (*m_data)[m_hi] = data;
        } else {
            m_data->push_back(data);
        }

        ++m_hi;

        return *this;
    }

    Slice & slice(SizeType lo, SizeType hi) {
        PGZXB_DEBUG_ASSERT_EX("lo must be less than hi", lo <= hi);
        if (lo == hi) {
            m_hi = m_lo;
            return *this;
        }
        if (hi > count()) m_data->resize(m_lo + hi);
        m_lo = m_lo + lo;
        m_hi = m_lo - lo + hi;

        return *this;
    }

    Slice sliced(SizeType lo, SizeType hi) const {
        PGZXB_DEBUG_ASSERT_EX("lo must be less than hi", lo <= hi);

        Slice copy = *this;

        return copy.slice(lo, hi);
    }

    T & operator[] (SizeType i) {
        return m_data->operator[](m_lo + i);
    }

    const T & operator[] (SizeType i) const {
        return m_data->operator[](m_lo + i);
    }

    iterator begin() {
        return m_data->begin() + m_lo;
    }

    iterator end() {
        return m_data->begin() + m_hi;
    }

    const_iterator begin() const {
        return m_data->begin() + m_lo;
    }

    const_iterator end() const {
        return m_data->begin() + m_hi;
    }

    SizeType count() const { return m_hi - m_lo; }
    bool hasSameBaseData(const Slice & other) const { return m_data == other.m_data; }

private:
    std::shared_ptr<Vec> m_data;
    SizeType m_lo = 0, m_hi = 0; // [lo, hi)
};

PGBN_NAMESPACE_END
#endif // !PGBIGNUMBER_SLICE_H