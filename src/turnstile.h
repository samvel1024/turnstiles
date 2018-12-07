#ifndef SRC_TURNSTILE_H_
#define SRC_TURNSTILE_H_

#include <atomic>
#include <mutex>
#include <string>
#include <iostream>


using namespace std;


void assertion(bool condition, const char *msg) {
  if (!condition) {
    throw runtime_error(string("Broken invariant: ") + msg);
  }
}

mutex queue_lock;
mutex turnstile_lock;

class Turnstile;

class Mutex;

thread_local Turnstile *thread_turnstile = nullptr;
thread_local int thread_id = -1;
atomic<int> thread_count;
atomic<int> turnstile_count;
atomic<int> mutex_count;

Turnstile *stack_head;

class Turnstile {
public:
  Turnstile *next = nullptr;
  const int id = turnstile_count++;
  mutex mutex;
  atomic<int> waiting_count;


  friend ostream &operator<<(ostream &os, const Turnstile &t) {
    os << "Turnstile (" << t.id << ") ";
    return os;
  }

};

class Mutex {
private:
  const int id = mutex_count++;
  Turnstile *current = nullptr;
public:

  Mutex() = default;

  Mutex(const Mutex &) = delete;

  Turnstile *getThreadTurnstile() {
    if (thread_id == -1){
      thread_id = thread_count++;
    }
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
    assertion(t->waiting_count == 0, "Busy turnstile is being inserted to free list");
    stack_head = t;
  }


  void lock() {

    {
      lock_guard<mutex> lk(turnstile_lock);
      if (current == nullptr) {
        current = getThreadTurnstile();
      }
      current->waiting_count++;
    }
    current->mutex.lock();
  }

  void unlock() {
    lock_guard<mutex> lk(turnstile_lock);
    current->waiting_count--;
    if (current->waiting_count == 0){
      insertToQueue(current);
      current = nullptr;
    }
  }

  friend ostream &operator<<(ostream &os, const Mutex &t) {
    os << "Mutex (" << t.id << ") ";
    return os;
  }

};

#endif  // SRC_TURNSTILE_H_
