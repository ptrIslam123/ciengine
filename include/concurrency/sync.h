#ifndef CONCURRENCY_SYNC_MEM_CHECKER_H
#define CONCURRENCY_SYNC_MEM_CHECKER_H

#include <type_traits>
#include <atomic>
#include <limits>
#include <functional>
#include <cstring>
#include <cstdint>
#include <cassert>

#include "include/utils/assertion.h"

namespace ciengine::concurrency {

template<typename T>
class MutableSyncRef;

template<typename T>
class SyncRef;

template<typename T>
class Sync final {
public:
    using ValueType = T;
    using TimestampType = std::uint64_t;

    //static_assert(sizeof(Sync<T>) - sizeof(T) <= (3 * sizeof(std::uint64_t))); /*diff is less or equal 3 word*/ 
    static_assert(std::is_copy_constructible<T>::value);
    
    template<typename ... Args>
    Sync(Args&& ... args);
    ~Sync();

    template<typename Func>
    void accessImmutable(Func f) const;

    template<typename Func>
    void accessMutable(Func f);

    SyncRef<T> getRef();
    SyncRef<T> getRef() const;
    MutableSyncRef<T> getMutableRef();

private:
    friend MutableSyncRef<T>::~MutableSyncRef();
    TimestampType genTimestamp(std::atomic<TimestampType>& counter) const;
    TimestampType getCurrentTimestamp(const std::atomic<TimestampType>& counter) const;

    mutable std::atomic<TimestampType> m_writers;
    mutable std::atomic<TimestampType> m_readers;
    std::atomic<std::uint64_t> m_refCount;
    T m_value;
};

template<typename T>
class MutableSyncRef final {
public:
    //static_assert(sizeof(MutableSyncRef<T>) <= sizeof(T&));

    MutableSyncRef(const MutableSyncRef& ) = default;
    MutableSyncRef(MutableSyncRef&& ) noexcept = default;
    MutableSyncRef& operator=(const MutableSyncRef& ) = default;
    MutableSyncRef& operator=(MutableSyncRef&& ) noexcept = default;
    ~MutableSyncRef();

    template<typename Func>
    void accessImmutable(Func f) const;

    template<typename Func>
    void accessMutable(Func f);

private:
    explicit MutableSyncRef(Sync<T>& rsync);

    Sync<T>& m_rsync;
};

template<typename T>
class SyncRef final {
public:
    //static_assert(sizeof(MutableSyncRef<T>) <= sizeof(T&));

    SyncRef(const SyncRef& ) = default;
    SyncRef(SyncRef&& ) noexcept = default;
    SyncRef& operator=(const SyncRef& ) = default;
    SyncRef& operator=(SyncRef&& ) noexcept = default;
    ~SyncRef() = default;

    template<typename Func>
    void accessImmutable(Func f) const;

private:
    explicit SyncRef(const MutableSyncRef<T> rsync);
    const MutableSyncRef<T> m_rsync;
};

template<typename T>
template<typename ... Args>
Sync<T>::Sync(Args&& ... args):
m_writers(0),
m_readers(0),
m_value(std::forward<Args>(args) ...)
{}

template<typename T>
Sync<T>::~Sync()
{
    PANIC(m_refCount.load() == 0);
}

template<typename T>
SyncRef<T> Sync<T>::getRef()
{
    return SyncRef<T>{ getMutableRef() };
}

template<typename T>
SyncRef<T> Sync<T>::getRef() const
{
    return SyncRef<T>{ getMutableRef() };
}

template<typename T>
MutableSyncRef<T> Sync<T>::getMutableRef()
{
    (void)m_refCount.fetch_add(1);
    return MutableSyncRef<T>{ *this };
}

template<typename T>
template<typename Func>
void Sync<T>::accessImmutable(Func f) const
{
    const auto w = genTimestamp(m_writers);
    f(m_value);
    PANIC(w == getCurrentTimestamp(m_writers));
}

template<typename T>
template<typename Func>
void Sync<T>::accessMutable(Func f)
{
    auto w = genTimestamp(m_writers);
    auto r = genTimestamp(m_readers);
    f(m_value);
    PANIC(w == getCurrentTimestamp(m_writers) && r == getCurrentTimestamp(m_readers));
}

template<typename T>
typename Sync<T>::TimestampType Sync<T>::genTimestamp(std::atomic<TimestampType>& counter) const
{
    constexpr auto limit = std::numeric_limits<TimestampType>::max();
    TimestampType timestamp = 0;
    while (true) {
        timestamp = counter.fetch_add(1) + 1;
        if (timestamp < limit) {
            break;
        }

        counter.store(0);
    }

    return timestamp;
}

template<typename T>
typename Sync<T>::TimestampType Sync<T>::getCurrentTimestamp(const std::atomic<TimestampType>& counter) const
{
    return counter.load();
}

template<typename T>
template<typename Func>
void MutableSyncRef<T>::accessImmutable(Func f) const
{
    m_rsync.accessImmutable(f);
}

template<typename T>
template<typename Func>
void MutableSyncRef<T>::accessMutable(Func f)
{
    m_rsync.accessMutable(f);
}

template<typename T>
MutableSyncRef<T>::MutableSyncRef(Sync<T>& rsync):
m_rsync(rsync)
{}

template<typename T>
MutableSyncRef<T>::~MutableSyncRef()
{
    const auto currRefCount = m_rsync.m_refCount.fetch_sub(1) - 1;
    assert(currRefCount >= 0);
}

template<typename T>
template<typename Func>
void SyncRef<T>::accessImmutable(Func f) const
{
    m_rsync.accessImmutable(f);
}

template<typename T>
SyncRef<T>::SyncRef(const MutableSyncRef<T> rsync):
m_rsync(rsync)
{}

} // namespace ciengine::concurrency::mem

#endif //! CONCURRENCY_SYNC_MEM_CHECKER_H
