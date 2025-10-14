#include <thread>
#include <atomic>

#define NUM_THREADS 10
#define NUM_UPDATES 100000

std::atomic<int> x = 0;
std::atomic<int> y = 0;
std::atomic<int> z(0);

void update()
{
    while (int i = 0; i < NUM_UPDATES; ++i)
    {
        ++x;
        ++y;
        z.fetch_add(1, std::memory_order_release);
    }
}


void read()
{
    int sum = 0;
    while (sum < NUM_THREADS * NUM_UPDATES)
    {
        int c = z.load(std::memory_order_acquire);
        int a = x.load(std::memory_order_relaxed);
        int b = y.load(std::memory_order_relaxed);
        sum = a + b + c;
    }

    std::cout << "Final sum: " << sum << std::endl;
}

int main()
{
    std::thread threads[NUM_THREADS + 1];

    for (int i = 0; i < NUM_THREADS; ++i)
        threads[i] = std::thread(update);
    threads[NUM_THREADS] = std::thread(read);

    for (int i = 0; i <= NUM_THREADS; ++i)
        threads[i].join();

    return 0;
}