#pragma once

#include <queue>
#include <thread>
#include <vector>

#include <concurrentqueue.h>

#include <mcga/threading/base/delayed_queue_wrapper.hpp>
#include <mcga/threading/base/disallow_copy_and_move.hpp>
#include <mcga/threading/base/immediate_queue_wrapper.hpp>
#include <mcga/threading/base/loop_tick_duration.hpp>

namespace mcga::threading::base {

template<class Processor>
class EventLoop:
    private Processor,
    public base::DelayedQueueWrapper<Processor, typename Processor::Task>,
    public base::ImmediateQueueWrapper<Processor, typename Processor::Task> {
 public:
    using Task = typename Processor::Task;

    using Processor::Processor;

    MCGA_THREADING_DISALLOW_COPY_AND_MOVE(EventLoop);

    ~EventLoop() = default;

    std::size_t sizeApprox() const {
        return this->getImmediateQueueSize() + this->getDelayedQueueSize();
    }

    void start(volatile std::atomic_bool* running) {
        while (running->load()) {
            if (!this->executeDelayed(this)) {
                this->executeImmediate(this);
            }
            std::this_thread::sleep_for(base::loopTickDuration);
        }
    }
};

}  // namespace mcga::threading::base
