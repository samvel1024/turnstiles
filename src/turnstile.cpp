#include "turnstile.h"
#include "util.h"

//********************************************************************
//******************* Turnstile implementation ***********************
//********************************************************************

Turnstile *Turnstile::free_list;
std::mutex Turnstile::list_guard;

Turnstile *Turnstile::provide_turnstile() {
  std::lock_guard<std::mutex> lk(Turnstile::list_guard);
  if (Turnstile::free_list == nullptr) {
    return new Turnstile();
  }
  Turnstile *ans = Turnstile::free_list;
  Turnstile::free_list = Turnstile::free_list->next;
  ans->next = nullptr;
  return ans;
}

void Turnstile::add_to_list(Turnstile *t) {
  std::lock_guard<std::mutex> lk(list_guard);
  ASSERT(t->next == nullptr, "Turnstile has to be detached");
  t->next = Turnstile::free_list;
  Turnstile::free_list = t;
}

void Turnstile::add_waiting() { this->waiting_count++; }

void Turnstile::lock() { this->turnstile_mutex.lock(); }

bool Turnstile::unlock() {
  this->waiting_count--;
  if (this->waiting_count == 0) {
    this->turnstile_mutex.unlock();
    LOG(cout << "adding " << *(this) << " from " << *this << "to free list");
    Turnstile::add_to_list(this);
    return true;
  } else {
    LOG(cout << "notify next waiting from " << *this);
    this->turnstile_mutex.unlock();
    return false;
  }
}

std::ostream &operator<<(std::ostream &os, const Turnstile &t) {
  os << "Turnstile (" << &t << ") ";
  return os;
}

//********************************************************************
//******************* Mutex implementation ***************************
//********************************************************************

std::array<std::mutex, Mutex::LOCK_COUNT> Mutex::turnstile_guards = {};

size_t Mutex::map_ptr(void *ptr) {
  auto ptr_val = reinterpret_cast<int64_t>(ptr);
  LOG(cout << "PTR:" << ptr_val);
  size_t h1 = std::hash<int64_t>{}(ptr_val);
  size_t ans = h1 % LOCK_COUNT;
  ASSERT(ans < LOCK_COUNT, "Hashed index not in range");
  LOG(cout << "HASH:" << ans);
  return ans;
}

void Mutex::lock() {
  {
    std::lock_guard<std::mutex> lk(Mutex::turnstile_guards[map_ptr(this)]);
    if (current == nullptr) {
      this->current = Turnstile::provide_turnstile();
      LOG(cout << "new " << *current << "on " << *this);
    }
    this->current->add_waiting();
  }
  LOG(cout << "Trying to acquire lock " << *this << *(this->current));
  this->current->lock();
  LOG(cout << "entering " << *this << " with " << *(this->current));
}

void Mutex::unlock() {
  std::lock_guard<std::mutex> lk(Mutex::turnstile_guards[map_ptr(this)]);
  ASSERT(this->current != nullptr, "Mutex turnstile cannot be null on unlock")
  if (this->current->unlock()) {
    this->current = nullptr;
  }
}

std::ostream &operator<<(std::ostream &os, const Mutex &t) {
  os << "Mutex (" << &t << ") ";
  return os;
}
