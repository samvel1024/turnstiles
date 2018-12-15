#ifndef SRC_TURNSTILE_H_
#define SRC_TURNSTILE_H_

#include <atomic>
#include <mutex>
#include <string>
#include <iostream>
#include <vector>
#include <array>
#include <unordered_map>

class Turnstile {
private:
  static std::mutex queue_guard;
  static Turnstile *queue;

  Turnstile *next = nullptr;

  std::atomic<int> waiting_count{};

  std::mutex turnstile_mutex;

  static void add_to_queue(Turnstile *t);

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

  static std::array<std::mutex, LOCK_COUNT> turnstile_locks;

  size_t map_ptr(void *ptr);

public:
  Mutex() = default;

  Mutex(const Mutex &) = delete;

  void lock();

  void unlock();

  friend std::ostream &operator<<(std::ostream &os, const Mutex &t);
};

#endif  // SRC_TURNSTILE_H_
