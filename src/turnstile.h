#ifndef SRC_TURNSTILE_H_
#define SRC_TURNSTILE_H_

#include <atomic>
#include <mutex>
#include <string>
#include <iostream>



#ifdef DEBUG
#define DBG(x) x
#else
#define DBG(x)
#endif

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


struct CurrentThread {

  static void init() {
    if (thread_id == -1) {
      thread_id = thread_count++;
    }
  }

  friend ostream &operator<<(ostream &os, const CurrentThread &t) {
    os << "[Thread-" << thread_id << "] === ";
    return os;
  }
};


CurrentThread current_thread;

class Mutex {
private:
  const int id = mutex_count++;
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
    assertion(t->waiting_count == 0, "Busy turnstile is being inserted to free list");
    stack_head = t;
  }


  void lock() {
    CurrentThread::init();
    {
      lock_guard<mutex> lk(turnstile_lock);
      if (current == nullptr) {
        current = getThreadTurnstile();
        DBG(cout << current_thread << "new " << *current << "on " << *this << endl);
      }
      current->waiting_count++;
    }
    DBG(cout << current_thread << "Trying to acquire lock" << *this << *(this->current) << endl);
    current->mutex.lock();
    DBG(cout << current_thread << "entering " << *this << " with " << *(this->current) << endl);
  }

  void unlock() {
    lock_guard<mutex> lk(turnstile_lock);
    current->waiting_count--;
    if (current->waiting_count == 0) {
      current->mutex.unlock();
      insertToQueue(current);
      DBG(cout << current_thread << "adding " << *current << " from " << *this << "to free list" << endl);
      current = nullptr;
    } else{
      DBG(cout << current_thread << "notify next waiting from " << *this << *current << endl);
      current->mutex.unlock();
    }
  }

  friend ostream &operator<<(ostream &os, const Mutex &t) {
    os << "Mutex (" << t.id << ") ";
    return os;
  }

};

#endif  // SRC_TURNSTILE_H_
