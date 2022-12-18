#include <functional>
#include <iostream>

int main(int argc, char *argv[]) {
  std::function<int(int,int)> adder = [](int a, int b) {
    return a + b;
  };
  std::cout << "adder(1,2) -> " << adder(1,2) << std::endl;

  int x = 7, y = 1;
  auto fn = [&x, y]() { x += y; };
  y = 1000;
  fn();

  [x]() {
    std::cout << "x is " << x << std::endl;
    // x++;   // <- an error, because x is read-only copy of x in main
  }();
  std::cout << "x is still " << x << std::endl;

  [&x]() {
    // now we have captured x by reference
    x++;
    std::cout << "x is " << x << std::endl;
    x++;
  }();
  std::cout << "x is now " << x << std::endl;
  return 0;
}
