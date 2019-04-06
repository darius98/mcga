#pragma once

#include <mcga/threading/base/thread_pool_wrapper.hpp>

namespace mcga::threading::constructs {

template<class W>
class WorkerThreadPoolConstruct : public base::ThreadPoolWrapper<W> {
 public:
    using Task = typename W::Task;

    using base::ThreadPoolWrapper<W>::ThreadPoolWrapper;

    MCGA_THREADING_DISALLOW_COPY_AND_MOVE(WorkerThreadPoolConstruct);

    ~WorkerThreadPoolConstruct() = default;

    void enqueue(const Task& task) {
        this->nextThread()->enqueue(task);
    }

    void enqueue(Task&& task) {
        this->nextThread()->enqueue(std::move(task));
    }
};

}  // namespace mcga::threading::constructs
