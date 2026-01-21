#pragma once
#include <condition_variable>
#include <functional>
#include <mutex>
#include <queue>
#include <thread>
#include <vector>

class ThreadPool {
public:
  using Task = std::function<void(int)>; // takes client_fd

  explicit ThreadPool(size_t num_threads, Task handler);
  ~ThreadPool();

  void submit(int client_fd);

  ThreadPool(const ThreadPool &) = delete;
  ThreadPool &operator=(const ThreadPool &) = delete;

private:
  void workerLoop();

  Task handler;
  std::vector<std::thread> workers;
  std::queue<int> client_queue;
  std::mutex mtx;
  std::condition_variable cv;
  bool stop = false;
};