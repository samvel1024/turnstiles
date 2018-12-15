#ifndef SRC_UTIL_H_
#define SRC_UTIL_H_

#include <atomic>
#include <iostream>
#include <mutex>

namespace util {

class Logger {
 public:
  static Logger instance;
  static thread_local int thread_id;
  static std::atomic<int> thread_count;
  static std::mutex io_lock;

  static void init() {
    if (thread_id == 0) {
      thread_id = ++thread_count;
    }
  }

  friend std::ostream &operator<<(std::ostream &os, const Logger &t) {
    os << "[Thread-" << thread_id << "] - ";
    return os;
  }
};

Logger Logger::instance;
thread_local int Logger::thread_id;
std::atomic<int> Logger::thread_count;
std::mutex Logger::io_lock;

void assertion(bool condition, const char *msg) {
  if (!condition) {
    throw std::runtime_error(std::string("Broken invariant: ") + msg);
  }
}
}  // namespace util

#ifdef DEBUG
#define LOG(x)                                                \
  util::Logger::init();                                       \
                                                              \
  {                                                           \
    std::unique_lock<std::mutex> __lk(util::Logger::io_lock); \
                                                              \
    std::cout << util::Logger::instance;                      \
                                                              \
    x << std::endl;                                           \
  }
#define ASSERT(x, y) util::assertion(x, y);
#else
#define LOG(x)
#define ASSERT(x, y)
#endif

#endif  // SRC_UTIL_H_
