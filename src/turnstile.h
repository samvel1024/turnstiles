#ifndef SRC_TURNSTILE_H_
#define SRC_TURNSTILE_H_

#include <atomic>
#include <mutex>
#include <string>
#include <iostream>
#include <vector>
#include <array>
#include "util.h"
#include "unordered_map"


using namespace std;


mutex queue_lock;
mutex turnstile_lock;
class Turnstile;

class Mutex;

atomic<int> turnstile_count;
std::array<std::mutex, 255> turnstile_locks;

Turnstile *stack_head;



size_t map_ptr(void *ptr){
  long ptr_val = reinterpret_cast<long>(ptr);
  size_t h1 = std::hash<long>{}(ptr_val);
  size_t ans = h1 % 256;
  util::assertion(ans < 256 , "Hashed index not in range");
  return ans;
}



class Turnstile {
public:
  Turnstile *next = nullptr;
  const int id = turnstile_count++;
  mutex turnstile_queue;
  atomic<int> waiting_count{};


  friend ostream &operator<<(ostream &os, const Turnstile &t) {
    os << "Turnstile (" << t.id << ") ";
    return os;
  }

};

thread_local Turnstile *thread_turnstile;



class Mutex {
private:
#ifdef DEBUG
  int id = util::mutex_count++;
#endif
  Turnstile *current = nullptr;
public:

  Mutex() = default;

  Mutex(const Mutex &) = delete;

  Turnstile *getThreadTurnstile() {
    if (thread_turnstile == nullptr) {
      thread_turnstile =new  Turnstile();
    }
    return thread_turnstile;
  }

  void insertToQueue(Turnstile *t) {
    lock_guard<mutex> lk(queue_lock);
    t->next = stack_head;
    util::assertion(t->waiting_count == 0, "Busy turnstile is being inserted to free list");
    stack_head = t;
  }


  void lock() {
    {
      lock_guard<mutex> lk(turnstile_locks[map_ptr(this)]);
      if (current == nullptr) {
        current = getThreadTurnstile();
        LOG(cout << "new " << *current << "on " << *this);
      }
      current->waiting_count++;
    }
    LOG(cout << "Trying to acquire lock" << *this << *(this->current));
    current->turnstile_queue.lock();
    LOG(cout << "entering " << *this << " with " << *(this->current));
  }

  void unlock() {
    lock_guard<mutex> lk(turnstile_locks[map_ptr(this)]);
    current->waiting_count--;
    if (current->waiting_count == 0) {
      current->turnstile_queue.unlock();
      LOG(cout << "adding " << *current << " from " << *this << "to free list");
      current = nullptr;
    } else {
      LOG(cout << "notify next waiting from " << *this << *current);
      current->turnstile_queue.unlock();
    }
  }

#ifdef DEBUG

  friend ostream &operator<<(ostream &os, const Mutex &t) {
    os << "Mutex (" << t.id << ") ";
    return os;
  }

#endif

};

#endif  // SRC_TURNSTILE_H_
