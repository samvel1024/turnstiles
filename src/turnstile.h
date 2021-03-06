#ifndef SRC_TURNSTILE_H_
#define SRC_TURNSTILE_H_

#include <array>
#include <atomic>
#include <iostream>
#include <mutex>
#include <string>
#include <unordered_map>
#include <vector>

class Turnstile {
 private:
  static std::mutex list_guard;
  static Turnstile *free_list;

  Turnstile *next = nullptr;

  std::atomic<int> waiting_count{};

  std::mutex turnstile_mutex;

  static void add_to_list(Turnstile *t);

 public:
  Turnstile() = default;

  void lock();

  bool unlock();

  void add_waiting();

  static Turnstile *provide_turnstile();

  friend std::ostream &operator<<(std::ostream &os, const Turnstile &t);
};

class Mutex {
 private:
  static constexpr int LOCK_COUNT = 256;

  Turnstile *current = nullptr;

  static std::array<std::mutex, LOCK_COUNT> turnstile_guards;

  size_t map_ptr(void *ptr);

 public:
  Mutex() = default;

  Mutex(const Mutex &) = delete;

  void lock();

  void unlock();

  friend std::ostream &operator<<(std::ostream &os, const Mutex &t);
};

#endif  // SRC_TURNSTILE_H_
