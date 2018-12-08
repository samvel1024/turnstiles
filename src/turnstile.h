#ifndef SRC_TURNSTILE_H_
#define SRC_TURNSTILE_H_

#include <atomic>
#include <mutex>
#include <string>
#include <iostream>
#include "util.h"


using namespace std;


mutex queue_lock;
mutex turnstile_lock;

class Turnstile;

class Mutex;

atomic<int> turnstile_count;
atomic<int> mutex_count;

Turnstile *stack_head;

class Turnstile {
public:
  Turnstile *next = nullptr;
  const int id = turnstile_count++;
  mutex turnstile_queue;
  atomic<int> waiting_count;


  friend ostream &operator<<(ostream &os, const Turnstile &t) {
    os << "Turnstile (" << t.id << ") ";
    return os;
  }

};


class Mutex {
private:
  Turnstile *current = nullptr;
public:

  Mutex() = default;

  Mutex(const Mutex &) = delete;

  Turnstile *getThreadTurnstile() {

    lock_guard<mutex> lk(queue_lock);
    if (stack_head == nullptr) {
      return new Turnstile();
    }
    Turnstile *ans = stack_head;
    stack_head = stack_head->next;
    ans->next = nullptr;
    return ans;
  }

  void insertToQueue(Turnstile *t) {
    lock_guard<mutex> lk(queue_lock);
    t->next = stack_head;
    util::assertion(t->waiting_count == 0, "Busy turnstile is being inserted to free list");
    stack_head = t;
  }


  void lock() {
    {
      lock_guard<mutex> lk(turnstile_lock);
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
    lock_guard<mutex> lk(turnstile_lock);
    current->waiting_count--;
    if (current->waiting_count == 0) {
      current->turnstile_queue.unlock();
      insertToQueue(current);
      LOG(cout << "adding " << *current << " from " << *this << "to free list");
      current = nullptr;
    } else {
      LOG(cout << "notify next waiting from " << *this << *current);
      current->turnstile_queue.unlock();
    }
  }

  friend ostream &operator<<(ostream &os, const Mutex &t) {
    os << "Mutex (" << ") ";
    return os;
  }

};

#endif  // SRC_TURNSTILE_H_
