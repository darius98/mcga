#pragma once

#include <mcga/threading/base/disallow_copy_and_move.hpp>
#include <mcga/threading/base/loop_tick_duration.hpp>
#include <mcga/threading/base/immediate_queue_wrapper.hpp>
#include <mcga/threading/base/single_producer_immediate_queue_wrapper.hpp>

namespace mcga::threading::base {

template<class P, class ImmediateQueue = base::ImmediateQueueWrapper<P>>
class Worker: public ImmediateQueue {
 public:
    using Processor = P;
    using Task = typename Processor::Task;

    Worker() = default;

    MCGA_THREADING_DISALLOW_COPY_AND_MOVE(Worker);

    ~Worker() = default;

    size_t sizeApprox() const {
        return this->getImmediateQueueSize();
    }

    void start(volatile std::atomic_bool* running, Processor* processor) {
        while (running->load()) {
            while (this->executeImmediate(processor)) {
                std::this_thread::yield();
            }
            std::this_thread::sleep_for(base::loopTickDuration);
        }
    }
};

template<class P>
using SingleProducerWorker = Worker<P, SingleProducerImmediateQueueWrapper<P>>;

template<class Wrapper>
class WorkerConstruct : public Wrapper {
 public:
    using Task = typename Wrapper::Task;

    using Wrapper::Wrapper;

    MCGA_THREADING_DISALLOW_COPY_AND_MOVE(WorkerConstruct);

    ~WorkerConstruct() = default;

    void enqueue(Task task) {
        this->getWorker()->enqueue(std::move(task));
    }
};

}  // namespace mcga::threading::base
