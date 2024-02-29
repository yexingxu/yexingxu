#pragma once

#include <atomic>
#include <condition_variable>
#include <cstdint>
#include <functional>
#include <future>
#include <mutex>
#include <queue>
#include <thread>
#include <type_traits>
#include <utility>

namespace thread {

template <class T>
class TaskQueue {
 public:
  TaskQueue() = default;
  TaskQueue(const TaskQueue& other) = delete;
  TaskQueue(TaskQueue&& other) = default;
  ~TaskQueue() = default;

  bool empty() {
    std::lock_guard<std::mutex> lock(mtx_);
    return tasks_.empty();
  }

  void push(const T& item) {
    std::lock_guard<std::mutex> lock(mtx_);
    tasks_.push(item);
  }

  bool pop(T& item) {
    if (empty()) {
      return false;
    }

    {
      std::lock_guard<std::mutex> lock(mtx_);
      item = std::move(tasks_.front());
      tasks_.pop();
    }
    return true;
  }

 private:
  std::queue<T> tasks_;
  std::mutex mtx_;
};

class ThreadPool {
 private:
  class ThreadWorker {
   public:
    ThreadWorker(ThreadPool* tp) : pool_(tp) {}
    void operator()() {
      std::function<void()> func;
      bool pop = false;
      while (!pool_->shutdown_) {
        // 为线程环境加锁，互访问工作线程的休眠和唤醒
        std::unique_lock<std::mutex> lock(pool_->mtx_);
        // 如果任务队列为空，阻塞当前线程
        if (pool_->taskQueue_.empty()) {
          pool_->cond_.wait(lock);  // 等待条件变量通知，开启线程 wait
                                    // 函数只能传入 unique_lock
        }
        pop = pool_->taskQueue_.pop(func);
        if (pop) {
          func();
        }
      }
    }

   private:
    ThreadPool* pool_;
  };

 public:
  ThreadPool(std::uint32_t capacity) : threads_(capacity), shutdown_(false) {
    for (std::uint32_t i = 0; i < capacity; ++i) {
      threads_.at(i) = std::thread(ThreadWorker(this));
    }
  }

  // Waits until threads finish their current task and shutdowns the pool
  ~ThreadPool() {
    shutdown_ = true;
    cond_.notify_all();  // 通知，唤醒所有工作线程
    for (auto& thread : threads_) {
      if (thread.joinable()) {  // 判断线程是否在等待
        thread.join();          // 将线程加入到等待队列
      }
    }
  }

  template <typename F, typename... Args>
  auto push(F&& f, Args&&... args) -> std::future<decltype(f(args...))> {
    // Create a function with bounded parameter ready to execute
    std::function<decltype(f(args...))()> func = std::bind(
        std::forward<F>(f),
        std::forward<Args>(
            args)...);  // 连接函数和参数定义，特殊函数类型，避免左右值错误

    // Encapsulate it into a shared pointer in order to be able to copy
    // construct
    auto task =
        std::make_shared<std::packaged_task<decltype(f(args...))()>>(func);

    auto lambda = [task]() { (*task)(); };

    taskQueue_.push(lambda);
    cond_.notify_one();
    return task->get_future();
  }

 private:
  TaskQueue<std::function<void()>> taskQueue_;
  std::vector<std::thread> threads_;
  std::atomic_bool shutdown_;
  std::mutex mtx_;
  std::condition_variable cond_;
};

}  // namespace thread