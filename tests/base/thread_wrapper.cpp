#pragma ide diagnostic ignored "readability-magic-numbers"

#include <vector>

#include <mcga/test_ext/matchers.hpp>
#include <mcga/test.hpp>

#include <mcga/threading.hpp>

#include "../testing_utils/rand_utils.hpp"

using mcga::matchers::isFalse;
using mcga::matchers::isTrue;
using mcga::test::expect;
using mcga::test::multiRunTest;
using mcga::test::test;
using mcga::test::TestConfig;
using mcga::threading::base::ThreadWrapper;
using mcga::threading::testing::randomBool;
using std::atomic_bool;
using std::operator""ns;
using std::size_t;
using std::thread;
using std::vector;

namespace {

struct BasicWorker {
    using Processor = int;
    using Task = int;

    volatile int numSpins = 0;

    size_t size() const {
        return 0;
    }

    void start(volatile atomic_bool* running, Processor* /*processor*/) {
        while (running->load()) {
            numSpins += 1;
            std::this_thread::sleep_for(20ns);
        }
    }
};

}  // namespace

TEST_CASE(ThreadWrapper, "ThreadWrapper") {
    test("Starting and stopping thread wrapper", [&] {
        auto loop = new ThreadWrapper<BasicWorker>();
        expect(loop->isRunning(), isFalse);
        loop->start();
        expect(loop->isRunning(), isTrue);
        loop->stop();
        expect(loop->isRunning(), isFalse);
        delete loop;
    });

    multiRunTest(TestConfig("Concurrent starts and stops do not break the "
                            "ThreadWrapper")
                   .setTimeTicksLimit(10),
                 10,
                 [&] {
                     constexpr int numWorkers = 100;
                     constexpr int numOps = 1000;

                     auto loop = new ThreadWrapper<BasicWorker>();

                     vector<thread*> workers(numWorkers, nullptr);
                     for (int i = 0; i < numWorkers; ++i) {
                         workers[i] = new thread([&] {
                             for (int j = 0; j < numOps; ++j) {
                                 if (randomBool()) {
                                     loop->start();
                                 } else {
                                     loop->stop();
                                 }
                             }
                         });
                     }
                     for (int i = 0; i < numWorkers; ++i) {
                         workers[i]->join();
                         delete workers[i];
                     }

                     delete loop;
                 });
}
