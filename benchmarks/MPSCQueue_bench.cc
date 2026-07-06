#include <thread>
#include <benchmark/benchmark.h>
#include <mmintrin.h>

#include "fiah/structs/MPSCQueue.hh"
#include "fiah/utils/Types.hh"

using namespace fiah;

static void BM_MPSCQueue_Push(benchmark::State &state)
{
    for (auto _ : state)
    {
        constexpr sz_t N = 1 << 10;
        MPSCQueue<int, N> queue;
        for (auto i{0uz}; i < N; ++i)
            (void)queue.try_push(static_cast<int>(i));

        benchmark::ClobberMemory();
    }
}

static void BM_MPSCQueue_PushAllThenPopAll(benchmark::State &state)
{


    for (auto _ : state)
    {
        constexpr sz_t N = 1 << 10;
        MPSCQueue<int, N> queue;
        const auto push_fn = [&queue] () {
            for (auto i{0uz}; i < N; ++i)
                (void)queue.try_push(static_cast<int>(i));
        };
        const auto pop_fn = [&queue] () {
            int out{};
            for (auto i{0uz}; i < N; ++i)
                (void)queue.try_pop(out);
        };

        std::thread producer{push_fn};
        std::thread consumer{pop_fn};
        producer.join();
        consumer.join();

        benchmark::ClobberMemory();
    }
}

static void BM_MPSCQueue_PushOnePopOne(benchmark::State &state)
{

    for (auto _ : state)
    {
        constexpr sz_t N = 1 << 10;
        MPSCQueue<int, N> queue;
        const auto push_fn = [&queue] () {
            for (auto i{0uz}; i < N; ++i)
                (void)queue.try_push(static_cast<int>(i));
        };

        std::thread producer{push_fn};

        int out{};
        sz_t popped{};
        while (popped < N)
        {
            if (queue.try_pop(out))
                ++popped;
        }

        producer.join();
        benchmark::ClobberMemory();
    }
}

static void BM_MPSCQueue_ManyProducersOneConsumer(benchmark::State &state)
{
    constexpr sz_t N = 1 << 10;
    constexpr sz_t N_PER_PRODUCER = N >> 2;

    MPSCQueue<int, N> queue;
    const auto push_fn = [&queue] () {
        for (auto i{0uz}; i < N_PER_PRODUCER; ++i)
            while(!queue.try_push(static_cast<int>(i)))
                _mm_pause();
    };

    for (auto _ : state)
    {
        std::thread producer1{push_fn};
        std::thread producer2{push_fn};
        std::thread producer3{push_fn};
        std::thread producer4{push_fn};

        int out{};
        sz_t popped{};
        while (popped < N)
        {
            if (queue.try_pop(out))
                ++popped;
        }

        producer1.join();
        producer2.join();
        producer3.join();
        producer4.join();
        benchmark::ClobberMemory();
    }
}

BENCHMARK(BM_MPSCQueue_Push);
BENCHMARK(BM_MPSCQueue_PushAllThenPopAll);
BENCHMARK(BM_MPSCQueue_PushOnePopOne);
BENCHMARK(BM_MPSCQueue_ManyProducersOneConsumer);
BENCHMARK_MAIN();