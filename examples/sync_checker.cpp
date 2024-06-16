#include <iostream>
#include <string>
#include <thread>
#include <chrono>
#include <random>

#include "include/concurrency/sync.h"

struct Foo {
    explicit Foo(int i = 0, std::string&& s = std::string{"None"}):
    iData(i), sData(s)
    {}

    int iData;
    std::string sData;
};

void DataRaiseTest()
{
    constexpr auto size = 1000;
    ciengine::concurrency::Sync<Foo> foo;

    std::thread writerThread([size, rfoo = foo.getRef()] mutable {
        for (auto i = 0; i < size; ++i) {
            const auto randVal = rand();
            rfoo.setValue(Foo { randVal, std::string{"String data "} + std::to_string(randVal) });
        }
    });

    std::thread readerThread([size, rfoo = foo.getRef()]{
        for (auto i = 0; i < size; ++i) {
            rfoo.readOnlyAccess([](const Foo& f) {
                std::cout << f.iData << " : " << f.sData << std::endl;
            });
        }
    });

    writerThread.join();
    readerThread.join();
}


void LifeTimeSafeTest()
{
    auto lambda = []{
        ciengine::concurrency::Sync<Foo> foo;
        return foo.getRef();
    };

    auto rfoo = lambda();
    rfoo.readOnlyAccess([](const Foo& f) {
       std::cout << f.iData << " : " << f.sData << std::endl;
    });
}

void OkTest()
{
    ciengine::concurrency::Sync<Foo> foo;
    auto rfoo = foo.getRef();
    rfoo.setValue(Foo {1023, "Hello world!"});

    rfoo.readOnlyAccess([](const Foo& f) {
        std::cout << "str=" << f.sData << std::endl;
    });
}

int main()
{

    return 0;
}
