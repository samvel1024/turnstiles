#ifndef SRC_TURNSTILE_H_
#define SRC_TURNSTILE_H_

#include <atomic>
#include <mutex>
#include <string>
#include <iostream>
#include <vector>
#include <array>
#include <unordered_map>

using namespace std;

class Turnstile {
private:

  static thread_local Turnstile *thread_turnstile;

public:

  mutex turnstile_mutex;

  atomic<int> waiting_count{};

  Turnstile() = default;

  static Turnstile *for_thread();

  friend ostream &operator<<(ostream &os, const Turnstile &t);

};


class Mutex {
private:

  static constexpr int LOCK_COUNT = 1;

  Turnstile *current = nullptr;

  static std::array<std::mutex, LOCK_COUNT> turnstile_locks;

  size_t map_ptr(void *ptr);

public:

  Mutex() = default;

  Mutex(const Mutex &) = delete;

  void lock();

  void unlock();

  friend ostream &operator<<(ostream &os, const Mutex &t);

};

#endif  // SRC_TURNSTILE_H_
