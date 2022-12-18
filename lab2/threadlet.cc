/**
 * File: threadlet.cc
 * ------------------
 */

#include <iostream>
#include <string>
#include "stack.hh"

/* forward declaration */
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

/*
 * We define the initial threadlet to run within the stack segment
 * provided by the process. All others need to be created using the
 * two argument Thread constructor, because they need to create their
 * own stacks.
 */
static Threadlet *current = new Threadlet("initial thread");

void Threadlet::run() {
  Threadlet *prev = current;
  current = this;
  std::cout << "prev->sp = " << prev->sp << ", next->sp = "
            << current->sp << std::endl;
  stack_switch(&prev->sp, &this->sp);
  std::cout << "prev->sp = " << prev->sp << ", next->sp = "
            << current->sp << ", this->sp = "<< this->sp << std::endl;
}

int main(int argc, char *argv[]) {
  Threadlet *one = new Threadlet(1, current);
  Threadlet *two = new Threadlet(2, one);
  current->next = two; // create small circular list of threads
  
  current->yield();
  std::cout << "back in " << current->name << "." << std::endl;
  current->yield();
  return 0;
}

static void threadlet_routine() {
  std::cout << "running " << current->name
            << " for first time." << std::endl;
  current->yield();
  std::cout << "running " << current->name
            << " after allowing peers to run." << std::endl;
  current->yield();
}
