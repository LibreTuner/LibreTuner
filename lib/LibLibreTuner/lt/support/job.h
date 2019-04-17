#ifndef LT_JOB_H
#define LT_JOB_H

#include <vector>
#include <future>
#include <thread>

#include "event.h"

namespace lt {

class JobControl;

class Job : public std::enable_shared_from_this<Job> {
public:
    friend JobControl;

    template<typename F, class ...Args>
    void run(F &&f, Args &&...args);

    inline bool running() const noexcept { return running_; }

    inline void cancel() noexcept { canceled_ = true; eventCanceled_(); }

    template<typename F>
    Event<>::ConnectionPtr onCanceled(F &&f) noexcept {
        return eventCanceled_.connect(std::forward<F>(f));
    }

    // Called when progress changes
    template<typename F>
    Event<double>::ConnectionPtr onProgress(F &&f) noexcept {
        return eventProgress_.connect(std::forward<F>(f));
    }

    // Returns current job progress, a ratio between 0.0 and 1.0
    inline double progress() const noexcept { return progress_; }

private:
    std::thread thread_;
    std::atomic<bool> running_{false};
    std::atomic<bool> canceled_{false};
    Event<> eventCanceled_;
    Event<double> eventProgress_;

    double progress_{0};
};

using JobPtr = std::shared_ptr<Job>;

class JobControl {
public:
    explicit JobControl(JobPtr job) : job_(std::move(job)) {}

    JobControl(const JobControl&) = delete;
    JobControl &operator=(const JobControl&) = delete;

    inline bool canceled() const noexcept { return job_->canceled_; }

    void setProgress(double progress) noexcept;

private:
    JobPtr job_;
};

class JobPool {
public:

private:
    std::vector<JobPtr> jobs_;
};




template<typename F, class... Args>
void Job::run(F &&f, Args &&... args) {
    if (running_) {
        throw std::runtime_error("run() called on active job");
    }
    std::packaged_task task(std::forward<F>(f));
    running_ = true;
    thread_ = std::thread([&, task{std::move(task)}] {
        task(JobControl(shared_from_this()), std::forward<Args>(args)...);
        running_ = false;
    });
}



}


#endif //LIBRETUNER_JOB_H
