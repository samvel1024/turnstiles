#include "turnstile.h"

#include <thread>

template <typename M>
class MutexCounter {
 public:
  volatile int count = 0;
  M m;

  MutexCounter() = default;

  MutexCounter(const MutexCounter &p) = default;

  void increment() {
    std::lock_guard<M> lk(m);
    int c = count;
    c = c + 1;
    count = c;
  }
};

#ifdef WITH_STD_MUTEX
bool std_mutex = true;
using Counter = MutexCounter<std::mutex>;
#else
bool std_mutex = false;
using Counter = MutexCounter<Mutex>;
#endif

int main() {
  srand(static_cast<unsigned int>(time(0)));

  size_t objects;
  size_t threads;
  int incremenet_per_thread;

  std::cout << "Using std mutex: " << std_mutex << std::endl;

  std::cin >> objects;
  std::cin >> threads;
  std::cin >> incremenet_per_thread;
  std::cout << "Number of guarded objects " << objects << std::endl;
  std::cout << "Number of threads " << threads << std::endl;
  std::cout << "Number of increments per thread " << incremenet_per_thread
            << std::endl;

  int expected_sum = static_cast<int>(incremenet_per_thread * threads);
  Counter **counters = new Counter *[objects];
  std::vector<std::thread> thread_list;

  // Initialize locked objects
  for (unsigned i = 0; i < objects; ++i) {
    counters[i] = new Counter();
  }

  thread_list.reserve(threads);
  for (unsigned i = 0; i < threads; ++i) {
    thread_list.emplace_back([&]() {
      for (int j = 0; j < incremenet_per_thread; ++j) {
        counters[rand() % objects]->increment();
      }
    });
  }

  for (auto &t : thread_list) t.join();

  int64_t sum = 0;
  for (unsigned i = 0; i < objects; ++i) {
    sum += counters[i]->count;
  }

  if (expected_sum != sum) {
    std::cerr << "ERROR: expected: " << expected_sum << " actual:" << sum
              << std::endl;
    return 1;
  }
  return 0;
}
