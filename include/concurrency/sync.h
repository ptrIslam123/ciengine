#ifndef CONCURRENCY_TRAINTS_SYNC_H
#define CONCURRENCY_TRAINTS_SYNC_H

#include <atomic>
#include <functional>

#include "include/concurrency/checker/mem_checker.h"
#include "include/utils/assertion.h"

namespace ciengine::concurrency {

template<typename T>
class SyncRef;

template<typename T>
class Sync final {
public:
    using ValueType = T;
    using LifeTimeCounterType = std::uint64_t;
    using RefType = SyncRef<T>;
    using ReadOnlyAcceccFuncType = typename mem::Checker<T>::ReadOnlyAcceccFuncType;
    using AcceccFuncType = typename mem::Checker<T>::AcceccFuncType;

    template<typename ... Args>
    Sync(Args&& ... args);
    ~Sync();

    Sync(const Sync& ) = delete;
    Sync(Sync&& ) noexcept = delete;
    Sync& operator=(const Sync& ) = delete;
    Sync& operator=(Sync&& ) noexcept = delete;

    void setValue(const T& newValue);
    void setValue(T&& newValue);

    void readOnlyAccess(ReadOnlyAcceccFuncType func);
    void access(AcceccFuncType func);

    T getValue();
    T getValue() const;

    RefType getRef();
    const RefType getRef() const;

private:
    friend SyncRef<T>;

    mem::Checker<T> m_checker;
    mutable std::atomic<LifeTimeCounterType> m_refCount;
};

template<typename T>
class SyncRef final {
public:
    using ValueType = T;
    using ReadOnlyAcceccFuncType = typename Sync<T>::ReadOnlyAcceccFuncType;
    using AcceccFuncType = typename Sync<T>::AcceccFuncType;

    SyncRef& operator=(const SyncRef& other) = delete;
    SyncRef& operator=(SyncRef&& other) noexcept = delete;

    SyncRef(const SyncRef& other);
    SyncRef(SyncRef&& other) noexcept;
    ~SyncRef();

    void setValue(const T& newValue);
    void setValue(T&& newValue);

    void readOnlyAccess(ReadOnlyAcceccFuncType func) const;
    void access(AcceccFuncType func);

    T getValue();
    T getValue() const;

private:
    friend Sync<T>;
    explicit SyncRef(Sync<T>& owner);

    Sync<T>& m_owner;
};

template<typename T>
template<typename ... Args>
Sync<T>::Sync(Args&& ... args):
m_checker(std::forward<Args>(args) ...)
{}

template<typename T>
Sync<T>::~Sync()
{
    PANIC(m_refCount.fetch_sub(1) - 1 != 0);
}

template<typename T>
void Sync<T>::setValue(const T& newValue)
{
    m_checker.write(newValue);
}

template<typename T>
void Sync<T>::setValue(T&& newValue)
{
    m_checker.write(std::move(newValue));
}

template<typename T>
void Sync<T>::readOnlyAccess(ReadOnlyAcceccFuncType func)
{
    m_checker.readOnlyAccess(func);
}

template<typename T>
void Sync<T>::access(AcceccFuncType func)
{
    m_checker.access(func);
}

template<typename T>
T Sync<T>::getValue()
{
    return m_checker.read();
}

template<typename T>
T Sync<T>::getValue() const
{
    return m_checker.read();
}

template<typename T>
typename Sync<T>::RefType Sync<T>::getRef()
{
    (void)m_refCount.fetch_add(1);
    return RefType{ *this };
}

template<typename T>
const typename Sync<T>::RefType Sync<T>::getRef() const
{
    (void)m_refCount.fetch_add(1);
    return RefType{ *this };
}


///////////////////////////////////////////////


template<typename T>
SyncRef<T>::SyncRef(Sync<T>& owner):
m_owner(owner)
{}

template<typename T>
SyncRef<T>::SyncRef(const SyncRef& other):
m_owner(other.m_owner)
{
    (void)m_owner.m_refCount.fetch_add(1);
}

template<typename T>
SyncRef<T>::SyncRef(SyncRef&& other) noexcept:
m_owner(other.m_owner)
{
    (void)m_owner.m_refCount.fetch_add(1);
}

template<typename T>
SyncRef<T>::~SyncRef()
{
    (void*)m_owner.m_refCount.fetch_sub(1);
}

template<typename T>
void SyncRef<T>::setValue(const T& newValue)
{
    m_owner.setValue(newValue);
}

template<typename T>
void SyncRef<T>::setValue(T&& newValue)
{
    m_owner.setValue(std::move(newValue));
}

template<typename T>
void SyncRef<T>::readOnlyAccess(ReadOnlyAcceccFuncType func) const
{
    m_owner.readOnlyAccess(func);
}

template<typename T>
void SyncRef<T>::access(AcceccFuncType func)
{
    m_owner.access(func);
}

template<typename T>
T SyncRef<T>::getValue()
{
    return m_owner.getValue();
}

template<typename T>
T SyncRef<T>::getValue() const
{
    return m_owner.getValue();
}

} //! namespace ciengine::concurrency

#endif //! CONCURRENCY_TRAINTS_SYNC_H
