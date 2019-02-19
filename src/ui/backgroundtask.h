#ifndef BACKGROUNDTASK_H
#define BACKGROUNDTASK_H

#include <future>
#include <thread>
#include <atomic>
#include <future>

#include <QApplication>

/**
 * Runs a task in the background while updating Qt
 */
template <typename T> class BackgroundTask;
template <class R, class ...Args>
class BackgroundTask<R(Args...)>
{
public:
    template<class F>
    explicit BackgroundTask(F &&f) : task_(std::move(f)) {}
    
    // Executes the function
    void operator()(Args && ...args) {
        std::atomic<bool> complete{false};
        std::thread worker([this, &complete](Args && ...args) {
            task_(std::forward<Args>(args)...);
            complete = true;
        }, std::forward<Args>(args)...);
        
        while (!complete) {
            QApplication::processEvents(QEventLoop::WaitForMoreEvents);
        }
        worker.join();
    }
    
    // Returns the future. Can only be called once.
    std::future<R> future() { return task_.get_future(); }
    
private:
    std::packaged_task<R(Args...)> task_;
};



#endif // BACKGROUNDTASK_H
