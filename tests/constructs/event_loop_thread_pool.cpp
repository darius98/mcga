#pragma ide diagnostic ignored "readability-magic-numbers"

#include <mutex>
#include <set>
#include <vector>

#include <mcga/test_ext/matchers.hpp>
#include <mcga/test.hpp>

#include <mcga/threading.hpp>

#include "../testing_utils/basic_processor.hpp"
#include "../testing_utils/rand_utils.hpp"

using mcga::matchers::eachElement;
using mcga::matchers::hasSize;
using mcga::matchers::isEqualTo;
using mcga::matchers::isNotEqualTo;
using mcga::test::expect;
using mcga::test::multiRunTest;
using mcga::test::setUp;
using mcga::test::tearDown;
using mcga::threading::constructs::EventLoopThreadPoolConstruct;
using mcga::threading::testing::BasicProcessor;
using mcga::threading::testing::randomBool;
using mcga::threading::testing::randomDelay;
using std::atomic_int;
using std::hash;
using std::operator""ms;
using std::thread;
using std::vector;
namespace this_thread = std::this_thread;

using TestingProcessor = BasicProcessor<int>;
using EventLoopThreadPool = EventLoopThreadPoolConstruct<TestingProcessor>;

TEST_CASE(EventLoopThreadPool, "EventLoopThreadPool") {
    EventLoopThreadPool* pool = nullptr;

    setUp([&] {
        pool = new EventLoopThreadPool(EventLoopThreadPool::NumThreads(3));
        pool->start();
    });

    tearDown([&] {
        TestingProcessor::reset();
        pool->stop();
        delete pool;
        pool = nullptr;
    });

    multiRunTest(
      "Tasks enqueued in a EventLoopThreadPool are executed on "
      "multiple threads, different from the main thread",
      10,
      [&] {
          constexpr int numTasks = 100000;

          for (int i = 0; i < numTasks; ++i) {
              pool->enqueue(1);
              pool->enqueueDelayed(1, 3ms);
          }

          while (TestingProcessor::numProcessed() != 2 * numTasks) {
              this_thread::sleep_for(1ms);
          }
          this_thread::sleep_for(100ms);

          expect(TestingProcessor::numProcessed(), isEqualTo(2 * numTasks));
          expect(TestingProcessor::threadIds, hasSize(3));
          expect(TestingProcessor::threadIds,
                 eachElement(
                   isNotEqualTo(hash<thread::id>()(this_thread::get_id()))));
      });

    multiRunTest("Tasks enqueued from multiple threads in a EventLoopThreadPool"
                 " are executed on multiple threads, different from the main "
                 "thread",
                 10,
                 [&] {
                     constexpr int numWorkers = 100;
                     constexpr int numWorkerJobs = 1000;

                     vector<thread*> workers(numWorkers, nullptr);
                     for (int i = 0; i < numWorkers; ++i) {
                         workers[i] = new thread([&] {
                             for (int j = 0; j < numWorkerJobs; ++j) {
                                 if (randomBool()) {
                                     pool->enqueueDelayed(1, randomDelay());
                                 } else {
                                     pool->enqueue(1);
                                 }
                             }
                         });
                     }
                     for (int i = 0; i < numWorkers; ++i) {
                         workers[i]->join();
                         delete workers[i];
                     }

                     while (TestingProcessor::numProcessed()
                            != numWorkers * numWorkerJobs) {
                         this_thread::sleep_for(1ms);
                     }
                     this_thread::sleep_for(100ms);

                     expect(TestingProcessor::numProcessed(),
                            isEqualTo(numWorkers * numWorkerJobs));
                     expect(TestingProcessor::threadIds, hasSize(3));
                     expect(TestingProcessor::threadIds,
                            eachElement(isNotEqualTo(
                              hash<thread::id>()(this_thread::get_id()))));
                 });
}
