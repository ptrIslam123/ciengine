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

namespace ciengine::concurrency::mem {

template<typename T>
class Checker final {
public:
    static_assert(std::is_copy_constructible<T>::value);

    using ValueType = T;
    using TimestampType = std::uint64_t;
    using ReadOnlyAcceccFuncType = std::function<void(const T&)>;
    using AcceccFuncType =std::function<void(T&)>;

    template<typename ... Args>
    Checker(Args&& ... args);

    void write(const T& newValue);
    void write(T&& newValue);

    T read();
    T read() const;

    void readOnlyAccess(ReadOnlyAcceccFuncType func);
    void access(AcceccFuncType func);

private:
    TimestampType genTimestamp(std::atomic<TimestampType>& counter);
    TimestampType getCurrentTimestamp(const std::atomic<TimestampType>& counter) const;

    std::atomic<TimestampType> m_writers;
    std::atomic<TimestampType> m_readers;
    T m_value;
};

template<typename T>
template<typename ... Args>
Checker<T>::Checker(Args&& ... args):
m_writers(0),
m_readers(0),
m_value(std::forward<Args>(args) ...)
{}

template<typename T>
void Checker<T>::write(T&& newValue)
{
    auto w = genTimestamp(m_writers);
    auto r = genTimestamp(m_readers);
    m_value = newValue;
    PANIC(w == getCurrentTimestamp(m_writers) && r == getCurrentTimestamp(m_readers));
}

template<typename T>
void Checker<T>::write(const T& newValue)
{
    auto w = genTimestamp(m_writers);
    auto r = genTimestamp(m_readers);
    m_value = newValue;
    PANIC(w == getCurrentTimestamp(m_writers) && r == getCurrentTimestamp(m_readers));
}

template<typename T>
T Checker<T>::read()
{
    auto w = genTimestamp(m_writers);
    auto copy = m_value;
    PANIC(w == getCurrentTimestamp(m_writers));
    return copy;
}

template<typename T>
T Checker<T>::read() const
{
    auto w = genTimestamp(m_writers);
    auto copy = m_value;
    PANIC(w == getCurrentTimestamp(m_writers));
}

template<typename T>
void Checker<T>::readOnlyAccess(ReadOnlyAcceccFuncType func)
{
    const auto w = genTimestamp(m_writers);
    func(m_value);
    PANIC(w == getCurrentTimestamp(m_writers));
}

template<typename T>
void Checker<T>::access(AcceccFuncType func)
{
    auto w = genTimestamp(m_writers);
    auto r = genTimestamp(m_readers);
    func(m_value);
    PANIC(w == getCurrentTimestamp(m_writers) && r == getCurrentTimestamp(m_readers));
}

template<typename T>
typename Checker<T>::TimestampType Checker<T>::genTimestamp(std::atomic<TimestampType>& counter)
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
typename Checker<T>::TimestampType Checker<T>::getCurrentTimestamp(const std::atomic<TimestampType>& counter) const
{
    return counter.load();
}

} // namespace ciengine::concurrency::mem


#endif //! CONCURRENCY_SYNC_MEM_CHECKER_H
