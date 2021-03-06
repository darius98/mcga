#pragma once

#include <queue>
#include <thread>
#include <vector>

#include "delayed_queue_wrapper.hpp"
#include "immediate_queue_wrapper.hpp"
#include "loop_tick_duration.hpp"
#include "sp_immediate_queue_wrapper.hpp"

namespace mcga::threading::base {

template<class P,
         class ImmediateQueue = base::ImmediateQueueWrapper<P>,
         class DelayedQueue = base::DelayedQueueWrapper<P>>
class EventLoop : public DelayedQueue, public ImmediateQueue {
  public:
    using Processor = P;
    using Task = typename Processor::Task;

  private:
    std::size_t sizeApprox() const {
        return this->getImmediateQueueSize() + this->getDelayedQueueSize();
    }

    void start(std::atomic_bool* running, Processor* processor) {
        while (running->load()) {
            while (this->executeDelayed(processor)
                   || this->executeImmediate(processor)) {
                std::this_thread::yield();
            }
            std::this_thread::sleep_for(base::loopTickDuration);
        }
    }

    template<class T>
    friend class ThreadWrapperBase;
};

template<class P>
using SPEventLoop = EventLoop<P, SPImmediateQueueWrapper<P>>;

template<class Wrapper>
class EventLoopConstruct : public Wrapper {
  public:
    using Task = typename Wrapper::Task;
    using Delay = typename Wrapper::Wrapped::Delay;
    using DelayedTaskPtr = typename Wrapper::Wrapped::DelayedTaskPtr;

    using Wrapper::Wrapper;

    void enqueue(Task task) {
        this->getWorker()->enqueue(std::move(task));
    }

    template<class Rep, class Ratio>
    DelayedTaskPtr
      enqueueDelayed(Task task,
                     const std::chrono::duration<Rep, Ratio>& delay) {
        return this->getWorker()->enqueueDelayed(
          std::move(task), std::chrono::duration_cast<Delay>(delay));
    }

    template<class Rep, class Ratio>
    DelayedTaskPtr
      enqueueInterval(Task task,
                      const std::chrono::duration<Rep, Ratio>& delay) {
        return this->getWorker()->enqueueInterval(
          std::move(task), std::chrono::duration_cast<Delay>(delay));
    }
};

}  // namespace mcga::threading::base

namespace mcga::threading {

template<class P>
using Enqueuer = base::EventLoop<P>;

template<class P>
using SPEnqueuer = base::SPEventLoop<P>;

}  // namespace mcga::threading
