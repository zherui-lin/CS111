#include <iostream>
#include "stack.hh"

static char alt_stack[8192];
static sp_t main_sp, alt_sp;

static void alt() {
  while (true) {
    std::cout << "I'm in alt." << std::endl;
    stack_switch(&alt_sp, &main_sp);
  }
}

int main(int argc, char *argv[]) {
  alt_sp = stack_init(alt_stack, sizeof(alt_stack), alt);
  for (size_t i = 0; i < 3; i++) {
    std::cout << "I'm in main." << std::endl;
    stack_switch(&main_sp, &alt_sp);
  }
  return 0;
}
