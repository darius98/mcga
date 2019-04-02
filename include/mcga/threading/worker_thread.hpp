#pragma once

#include <mutex>
#include <thread>
#include <vector>

#include <concurrentqueue.h>

#include "internal/thread_wrapper.hpp"

namespace mcga::threading {

class Worker {
 private:
    using Executable = std::function<void()>;

    Worker();
    ~Worker() = default;

    DISALLOW_COPY_AND_MOVE(Worker);

    size_t sizeApprox() const;

    void start(volatile std::atomic_bool* running);

    void enqueue(const Executable& job);
    void enqueue(Executable&& job);

    void run();

    moodycamel::ConcurrentQueue<Executable> queue;
    moodycamel::ConsumerToken queueConsumerToken;
    std::vector<Executable> queueBuffer;
    std::atomic_size_t numBuffered = 0;

friend class internal::ThreadWrapper<Worker>;
friend class WorkerThread;
};

class WorkerThread : public internal::ThreadWrapper<Worker> {
 public:
    using Executable = Worker::Executable;

    WorkerThread() = default;
    ~WorkerThread() = default;

    DISALLOW_COPY_AND_MOVE(WorkerThread);

    void enqueue(const Executable& func);
    void enqueue(Executable&& func);
};

}  // namespace mcga::threading