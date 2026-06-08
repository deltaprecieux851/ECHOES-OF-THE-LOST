#include "core/JobSystem.h"

#include <algorithm>

namespace echoes::core {

JobSystem::JobSystem(std::size_t workerCount) {
    if (workerCount == 0) {
        workerCount = std::max(1u, std::thread::hardware_concurrency() - 1u);
    }

    workers_.reserve(workerCount);
    for (std::size_t i = 0; i < workerCount; ++i) {
        workers_.emplace_back(&JobSystem::WorkerLoop, this);
    }
}

JobSystem::~JobSystem() {
    Shutdown();
}

void JobSystem::Submit(std::function<void()> job) {
    {
        std::lock_guard lock(mutex_);
        jobs_.push(std::move(job));
    }
    cv_.notify_one();
}

void JobSystem::WaitForCompletion() {
    while (activeJobs_.load() > 0 || !jobs_.empty()) {
        std::this_thread::yield();
    }
}

void JobSystem::Shutdown() {
    if (!running_.exchange(false)) {
        return;
    }

    cv_.notify_all();
    for (auto& worker : workers_) {
        if (worker.joinable()) {
            worker.join();
        }
    }
    workers_.clear();
}

void JobSystem::WorkerLoop() {
    while (running_) {
        std::function<void()> job;

        {
            std::unique_lock lock(mutex_);
            cv_.wait(lock, [this] {
                return !running_ || !jobs_.empty();
            });

            if (!running_ && jobs_.empty()) {
                return;
            }

            if (!jobs_.empty()) {
                job = std::move(jobs_.front());
                jobs_.pop();
            }
        }

        if (job) {
            ++activeJobs_;
            job();
            --activeJobs_;
        }
    }
}

}  // namespace echoes::core
