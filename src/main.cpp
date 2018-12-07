#include "turnstile.h"

#include <thread>
#include <vector>

template<typename M>
class MutexCounter {
public:
  volatile int count = 0;
  M m;

  MutexCounter() = default;

  MutexCounter(const MutexCounter &p) = default;

  void increment() {
    lock_guard<M> lk(m);
    int c = count;
    c = c + 1;
    count = c;
  }
};

#ifdef WITH_STD_MUTEX
using Counter = MutexCounter<std::mutex>;
#else
using Counter = MutexCounter<Mutex>;
#endif



int main() {

  srand((int) time(0));

  size_t objects;
  size_t threads;
  int incremenet_per_thread;

  cout << "Enter number of guarded objects" << endl;
  cin >> objects;
  cout << "Enter number of threads" << endl;
  cin >> threads;
  cout << "Enter number of increments per thread" << endl;
  cin >> incremenet_per_thread;

  int expected_sum = static_cast<int>(incremenet_per_thread * threads);
  Counter **counters = new Counter *[objects];
  vector<thread> thread_list;

  //Initialize locked objects
  for (int i = 0; i < objects; ++i) {
    counters[i] = new Counter();
  }


  thread_list.reserve(threads);
  for (int i = 0; i < threads; ++i) {
    thread_list.emplace_back([&]() {
      for (int j = 0; j < incremenet_per_thread; ++j) {
        counters[rand() % objects]->increment();
      }
    });
  }


  for (auto &t:thread_list)
    t.join();

  long sum = 0;
  for (int i = 0; i < objects; ++i) {
    sum += counters[i]->count;
  }


  for (int i = 0; i < objects; ++i) {
    cout << i << ": " << counters[i]->count << endl;
  }


  if (expected_sum != sum) {
    cerr << "ERROR: expected: " << expected_sum << " actual:" << sum << endl;
    return 1;
  }
  return 0;

}
