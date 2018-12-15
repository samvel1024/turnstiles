#include "turnstile.h"
#include "util.h"

using namespace std;


//********************************************************************
//******************* Turnstile implementation ***********************
//********************************************************************

thread_local Turnstile *Turnstile::thread_turnstile = nullptr;

Turnstile *Turnstile::for_thread() {
  if (thread_turnstile == nullptr) {
    thread_turnstile = new Turnstile();
  }
  return thread_turnstile;
}


ostream &operator<<(ostream &os, const Turnstile &t) {
  os << "Turnstile (" << &t << ") ";
  return os;
}

//********************************************************************
//******************* Mutex implementation ***************************
//********************************************************************

std::array<std::mutex, Mutex::LOCK_COUNT> Mutex::turnstile_locks = {};


size_t Mutex::map_ptr(void *ptr) {
  long ptr_val = reinterpret_cast<long>(ptr);
  LOG(cout << "PTR:" << ptr_val);
  size_t h1 = std::hash<long>{}(ptr_val);
  size_t ans = h1 % LOCK_COUNT;
  util::assertion(ans < LOCK_COUNT, "Hashed index not in range");
  LOG(cout << "HASH:" << ans);
  return ans;
}

void Mutex::lock() {
  {
    lock_guard<mutex> lk(Mutex::turnstile_locks[map_ptr(this)]);
    if (current == nullptr) {
      current = Turnstile::for_thread();
      LOG(cout << "new " << *current << "on " << *this);
    }
    current->waiting_count++;
  }
  LOG(cout << "Trying to acquire lock" << *this << *(this->current));
  current->turnstile_mutex.lock();
  LOG(cout << "entering " << *this << " with " << *(this->current));
}

void Mutex::unlock() {
  lock_guard<mutex> lk(Mutex::turnstile_locks[map_ptr(this)]);
  current->waiting_count--;
  if (current->waiting_count == 0) {
    current->turnstile_mutex.unlock();
    LOG(cout << "adding " << *current << " from " << *this << "to free list");
    current = nullptr;
  } else {
    LOG(cout << "notify next waiting from " << *this << *current);
    current->turnstile_mutex.unlock();
  }
}

ostream &operator<<(ostream &os, const Mutex &t) {
  os << "Mutex (" << &t << ") ";
  return os;
}
