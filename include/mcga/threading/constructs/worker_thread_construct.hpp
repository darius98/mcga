#pragma once

#include <mcga/threading/base/thread_wrapper.hpp>

namespace mcga::threading::constructs {

template<class W>
class WorkerThreadConstruct : public base::ThreadWrapper<W> {
 public:
    using Task = typename W::Task;

    using base::ThreadWrapper<W>::ThreadWrapper;

    MCGA_THREADING_DISALLOW_COPY_AND_MOVE(WorkerThreadConstruct);

    ~WorkerThreadConstruct() = default;

    void enqueue(Task task) {
        this->getWorker()->enqueue(std::move(task));
    }
};

}  // namespace mcga::threading::constructs
