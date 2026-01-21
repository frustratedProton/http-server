#include "thread_pool.hpp"
#include <unistd.h>

ThreadPool::ThreadPool(size_t num_threads, Task handler)
    : handler(std::move(handler)) {
  for (size_t i = 0; i < num_threads; i++) {
    workers.emplace_back(&ThreadPool::workerLoop, this);
  }
}

ThreadPool::~ThreadPool() {
  {
    std::lock_guard<std::mutex> lock(mtx);
    stop = true;
  }
  cv.notify_all();
  for (auto &worker : workers) {
    worker.join();
  }
}

void ThreadPool::submit(int client_fd) {
  {
    std::lock_guard<std::mutex> lock(mtx);
    client_queue.push(client_fd);
  }
  cv.notify_one();
}

void ThreadPool::workerLoop() {
  while (true) {
    int client_fd;
    {
      std::unique_lock<std::mutex> lock(mtx);
      cv.wait(lock, [this]() { return stop || !client_queue.empty(); });

      if (stop && client_queue.empty()) {
        return;
      }

      client_fd = client_queue.front();
      client_queue.pop();
    }

    handler(client_fd);
    close(client_fd);
  }
}