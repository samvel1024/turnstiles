#ifndef TURNSTILES_UTIL_H
#define TURNSTILES_UTIL_H

#include <iostream>
#include <atomic>
#include <mutex>

namespace util {

  thread_local int thread_id = -1;
  std::atomic<int> thread_count;

  struct CurrentThread {
    static void init() {
      if (thread_id == -1) {
        thread_id = thread_count++;
      }
    }

    friend std::ostream &operator<<(std::ostream &os, const CurrentThread &t) {
      os << "[Thread-" << thread_id << "] - ";
      return os;
    }
  };

  CurrentThread current_thread;

  std::mutex io_lock;

  void assertion(bool condition, const char *msg) {
    if (!condition) {
      throw std::runtime_error(std::string("Broken invariant: ") + msg);
    }
  }


}

#ifdef DEBUG
#define LOG(x) {\
util::CurrentThread::init(); \
std::unique_lock<std::mutex> lk(util::io_lock);\
cout << util::current_thread;\
x << endl;\
}
#else
#define LOG(x)
#endif

#endif //TURNSTILES_UTIL_H
