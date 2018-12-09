#ifndef TURNSTILES_UTIL_H
#define TURNSTILES_UTIL_H

#include <iostream>
#include <atomic>
#include <mutex>

namespace util {

  thread_local int thread_id = -1;
  std::atomic<int> thread_count;

#ifdef DEBUG
  constexpr bool is_debug = true;
#else
  constexpr bool is_debug = false;
#endif

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
  std::atomic<int> mutex_count;


  void assertion(bool condition, const char *msg) {
    if (!condition) {
      throw std::runtime_error(std::string("Broken invariant: ") + msg);
    }
  }

  template<typename T>
  struct node {
    T data;
    node *next;

    node(const T &data) : data(data), next(nullptr) {}
  };

  template<typename T>
  class stack {
    std::atomic<node<T> *> head;
  public:
    void push(const T &data) {
      node<T> *new_node = new node<T>(data);
      new_node->next = head.load();
      while (!head.compare_exchange_weak(new_node->next, new_node));
    }

    node<T> *get_head() { return head; }
  };

}

#ifdef DEBUG
#define LOG(x) util::CurrentThread::init(); \
{\
std::unique_lock<std::mutex> lk(util::io_lock);\
cout << util::current_thread;\
x << endl;\
}
#else
#define LOG(x)
#endif

#endif 
