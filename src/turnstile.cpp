#include "turnstile.h"
#include "util.h"

using namespace std;


//********************************************************************
//******************* Turnstile implementation ***********************
//********************************************************************

Turnstile *Turnstile::queue;
std::mutex Turnstile::queue_guard;

Turnstile *Turnstile::provide_turnstile() {
  std::lock_guard<std::mutex> lk(queue_guard);
  if (queue == nullptr){
    return new Turnstile();
  }
  Turnstile *ans = queue;
  queue = queue->next;
  ans -> next = nullptr;
  return ans;
}

void Turnstile::add_to_queue(Turnstile *t){
  std::lock_guard<std::mutex> lk(queue_guard);
  ASSERT(t->next == nullptr, "Turnstile has to be detached");
  t->next = queue;
  queue = t;
}

void Turnstile::lock(){
  waiting_count++;
  turnstile_mutex.lock();
}

bool Turnstile::unlock(){
  Turnstile::waiting_count--;
  if (Turnstile::waiting_count == 0) {
    Turnstile::turnstile_mutex.unlock();
    LOG(cout << "adding " << *(this) << " from " << *this << "to free list");
    Turnstile::add_to_queue(this);
    return true;
  } else {
    LOG(cout << "notify next waiting from " << *this );
    Turnstile::turnstile_mutex.unlock();
    return false;
  }
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
  ASSERT(ans < LOCK_COUNT, "Hashed index not in range");
  LOG(cout << "HASH:" << ans);
  return ans;
}

void Mutex::lock() {
  {
    lock_guard<mutex> lk(Mutex::turnstile_locks[map_ptr(this)]);
    if (current == nullptr) {
      this->current = Turnstile::provide_turnstile();
      LOG(cout << "new " << *current << "on " << *this);
    }
  }
  LOG(cout << "Trying to acquire lock " << *this << *(this->current));
  this->current->lock();
  LOG(cout << "entering " << *this << " with " << *(this->current));
}

void Mutex::unlock() {
  lock_guard<mutex> lk(Mutex::turnstile_locks[map_ptr(this)]);
  ASSERT(this->current != nullptr, "Mutex turnstile cannot be null on unlock")
  if (this->current->unlock()){
    this->current = nullptr;
  }
}

ostream &operator<<(ostream &os, const Mutex &t) {
  os << "Mutex (" << &t << ") ";
  return os;
}
