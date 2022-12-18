#include <chrono>
#include <iostream>
#include <string>
#include <poll.h>
#include "stack.hh"
#include "timer.hh"

static void threadlet_routine();

struct Threadlet {

  Threadlet(const std::string& name): 
    name(name) {} // all other fields are nullptr by default

  Threadlet(size_t num, Threadlet *next):
    name("thread #" + std::to_string(num)),
    stack(new char[kStackSize]),
    sp(stack_init(stack.get(), kStackSize, threadlet_routine)),
    next(next) {}
  
  void run();
  void yield() { next->run(); }
  
  std::string name;
  Bytes stack; // defaulty built around nullptr if otherwise uninitialized  
  sp_t sp = nullptr; // really a std::uintptr_t * (see stack.hh)
  Threadlet *next = nullptr; // neighbor in circular list of Threadlets
  static const size_t kStackSize = 8192;
};

Threadlet *current = new Threadlet("initial thread");

void Threadlet::run() {
  Threadlet *prev = current;
  current = this;
  stack_switch(&prev->sp, &this->sp);
}

std::atomic<int> good_counter;
int bad_counter;

void delay(int msec) {
  using clock = std::chrono::steady_clock;
  clock::time_point start = clock::now();
  while (clock::now() - start < std::chrono::milliseconds(msec)) {
    // IntrGuard guard;
    int old_value = bad_counter;
    good_counter++;
    bad_counter = old_value + 1;
  }
}

static void threadlet_routine() {
  intr_enable(true);
  for (size_t i = 0; i < 30; i++) {
    delay(10);
    std::cout << "running in " << current->name << std::endl;
  }
  std::cout << "good counter = " << good_counter << ", "
            << "bad counter = " << bad_counter << std::endl;
  exit(0);
}

static void timer_handler() {
  current->yield();
}

int main(int argc, char *argv[]) {
  Threadlet *one = new Threadlet(1, current);
  Threadlet *two = new Threadlet(2, one);
  current->next = two; // create small circular list of threads
  timer_init(100'000, timer_handler);
  threadlet_routine();
  return 0;
}
