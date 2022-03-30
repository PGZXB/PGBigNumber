#ifndef PGBIGNUMBER_INFIXEXPR_OBJMGR_H
#define PGBIGNUMBER_INFIXEXPR_OBJMGR_H

#include "../../include/PGBigNumber/fwd.h"
#include <unordered_map>
PGBN_NAMESPACE_START
template <typename T>
class ObjectManager {
public:
    ObjectManager() = default;

    ObjectManager(const ObjectManager &) = delete;
    ObjectManager(ObjectManager &&) noexcept = default;

    ObjectManager & operator=(const ObjectManager &) = delete;
    ObjectManager & operator=(ObjectManager &&) noexcept = default;

    void registe(const std::string & name, const T & obj) { // 覆盖
        m_objOfName[name] = obj;
    }

    const T * get(const std::string & name) const {
        auto iter = m_objOfName.find(name);

        if (iter == m_objOfName.end()) return nullptr;
        return &iter->second;
    }

    T * get(const std::string & name) {
        const ObjectManager * constThis = this;
        return const_cast<T*>(constThis->get(name));
    }

private:
    std::unordered_map<std::string, T> m_objOfName;
};
PGBN_NAMESPACE_END
#endif
