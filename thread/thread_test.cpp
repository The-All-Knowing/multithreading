#include <gtest/gtest.h>

#include <thread>


TEST(Thread, Basic)
{
    int x = 0;
    auto f = [&x]() {
        for (int i = 0; i < 1000; ++i)
            ++x;
    };

    std::thread t1(f);
    std::thread t2(f);

    t1.join();
    t2.join();

    EXPECT_EQ(x, 2000);
}