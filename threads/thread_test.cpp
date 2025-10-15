#include <gtest/gtest.h>

#include <chrono>
#include <iostream>
#include <thread>

#include "writter.hpp"


void f1(int n)
{
    Writer writer;
    for (int i = 0; i < 5; ++i)
    {
        ++n;
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
    writer << " f1: " << n << std::endl;
}

void f2(int& n)
{
    Writer writer;
    for (int i = 0; i < 5; ++i)
    {
        ++n;
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
    writer << " f2: " << n << std::endl;
}

class foo
{
    Writer writer;
public:
    foo() = default;
    foo(const foo&) : n(0), writer() {}
    foo(foo&&) noexcept : n(0), writer() {}

    void bar()
    {
        for (int i = 0; i < 5; ++i)
        {
            ++n;
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
        writer << " foo::bar: " << n << std::endl;
    }
    int n = 0;
};

class baz
{
    Writer writer;
public:
    baz() = default;
    baz(const baz&) : n(0), writer() {}
    baz(baz&&) noexcept : n(0), writer() {}

    void operator()()
    {
        for (int i = 0; i < 5; ++i)
        {
            ++n;
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
        writer << " baz::operator(): " << n << std::endl;
    }
    int n = 0;
};

TEST(Threads, BasicThreading)
{
    int n = 0;
    foo f;
    f.bar();
    baz b;
    
    std::thread t1(f1, n + 1);
    std::thread t2(f2, std::ref(n));
    std::thread t3(std::move(t2));
    std::thread t4(&foo::bar, &f);
    std::thread t5(&foo::bar, f);
    std::thread t6(b);

    t1.join();
    t3.join();
    t4.join();
    t5.join();
    t6.join();

    ASSERT_EQ(n, 5);
    ASSERT_EQ(f.n, 10);
    ASSERT_EQ(b.n, 0);
}