#include <gtest/gtest.h>

#include <latch>
#include <iostream>
#include <stop_token>
#include <thread>
#include <vector>

#include "writter.hpp"


TEST(JThreads, StopToken)
{
    std::jthread t1(
        [](std::stop_token st)
        {
            Writer writer;
            int n = 0;
            while (!st.stop_requested())
            {
                writer << "Thread working: " << n++ << std::endl;
                std::this_thread::sleep_for(std::chrono::milliseconds(10));
            }
            writer << "Thread stopping as requested." << std::endl;
        });

    std::this_thread::sleep_for(std::chrono::milliseconds(30));
    t1.request_stop();

    std::jthread t2(
        [](std::stop_token st)
        {
            Writer writer;
            int n = 0;
            while (!st.stop_requested())
            {
                writer << "Thread working: " << n++ << std::endl;
                std::this_thread::sleep_for(std::chrono::milliseconds(10));
            }
            writer << "Thread stopping as requested." << std::endl;
        });

    std::this_thread::sleep_for(std::chrono::milliseconds(50));
}

TEST(JThread, SharedStopToken)
{
    int countThreads = 5;
    std::latch sync_point(countThreads + 1);
    std::vector<std::jthread> stoppers(countThreads);

    std::jthread t(
        [](std::stop_token st)
        {
            Writer writer;
            int n = 0;
            while (!st.stop_requested())
            {
                writer << "Worker: " << n++ << std::endl;
                std::this_thread::sleep_for(std::chrono::milliseconds(20));
            }
            writer << "Worker stopping as requested." << std::endl;
        });

    auto st = t.get_stop_source();

    for (int i = 0; i < countThreads; ++i)
    {
        stoppers[i] = std::jthread(
            [st, &sync_point, i]()
            {
                Writer writer;
                sync_point.arrive_and_wait();
                st.request_stop();
                writer << "Stopper " << i << " requested stop" << std::endl;
            });
    }

    sync_point.arrive_and_wait();  // запустить всех одновременно

    ASSERT_TRUE(st.stop_requested());
}

TEST(JThread, StopCallback)
{
    std::mutex mutex;
    std::condition_variable_any cv;

    std::jthread worker([&](std::stop_token stoken)
    {
        Writer() << "Worker thread's id: " << std::this_thread::get_id() << '\n';
        std::unique_lock lock(mutex);
        cv.wait(lock, stoken, [&] { return stoken.stop_requested(); });
        Writer() << "Worker exiting\n";
    });

    std::stop_token token = worker.get_stop_token();
    ASSERT_FALSE(token.stop_requested());

    // Основной callback
    bool callback_executed = false;
    std::stop_callback cb(token, [&]
    {
        callback_executed = true;
        Writer() << "Stop callback executed by thread: "
                 << std::this_thread::get_id() << '\n';
    });

    // Временный callback (не должен сработать)
    {
        std::stop_callback temp_cb(token, []
        {
            Writer() << "This callback should NOT run\n";
        });
    }

    // Два потока, запрашивающих остановку
    auto stopper_func = [&worker]()
    {
        if (worker.request_stop())
            Writer() << "Stop request executed by thread: "
                     << std::this_thread::get_id() << '\n';
    };

    std::jthread stopper1(stopper_func);
    std::jthread stopper2(stopper_func);

    stopper1.join();
    stopper2.join();

    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    ASSERT_TRUE(token.stop_requested());
    ASSERT_TRUE(callback_executed);

    // Новый callback после запроса должен выполняться сразу
    bool immediate_cb = false;
    std::stop_callback cb2(token, [&] { immediate_cb = true; });
    ASSERT_TRUE(immediate_cb);
}