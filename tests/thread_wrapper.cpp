#pragma ide diagnostic ignored "readability-magic-numbers"

#include <random>
#include <vector>

#include <kktest.hpp>
#include <kktest_ext/matchers.hpp>

#include "mcga/threading/internal/thread_wrapper.hpp"

using kktest::test;
using mcga::threading::internal::ThreadWrapper;
using std::atomic_bool;
using std::operator""ns;
using std::size_t;
using std::thread;
using std::vector;

struct BasicWorker {
    atomic_bool running = false;
    volatile int numSpins = 0;

    size_t size() const {
        return 0;
    }

    void start() {
        if (!running.exchange(true)) {
            while (running) {
                numSpins += 1;
                std::this_thread::sleep_for(20ns);
            }
        }
    }

    void stop() {
        running = false;
    }
};

bool randomBool() {
    static std::random_device rd;
    static std::mt19937 generator(rd());
    static std::uniform_int_distribution<> distribution(0, 1);
    return distribution(generator) == 1;
}

TEST_CASE(ThreadWrapper, "ThreadWrapper") {
    test("Concurrent starts and stops do not break the ThreadWrapper", [&] {
        constexpr int numWorkers = 10;
        constexpr int numOps = 500;

        auto loop = new ThreadWrapper<BasicWorker>();

        vector<thread*> workers(numWorkers, nullptr);
        for (int i = 0; i < numWorkers; ++ i) {
            workers[i] = new thread([&] {
                for (int j = 0; j < numOps; ++ j) {
                    if (randomBool()) {
                        loop->start();
                    } else {
                        loop->stop();
                    }
                }
            });
        }
        for (int i = 0; i < numWorkers; ++ i) {
            workers[i]->join();
            delete workers[i];
        }
    });
}