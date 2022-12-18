#include <atomic>
#include <sstream>
#include <thread>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

/**
 * Function: delay
 * ---------------
 * Stall the calling thread for at least the specified
 * number of milliseconds.
 */
static void delay(int msec) {
  using clock = std::chrono::steady_clock;
  clock::time_point start = clock::now();
  while (clock::now() - start < std::chrono::milliseconds(msec)) ;
}

static void count(const std::string& name, std::atomic<int>& value) {
  for (size_t i = 0; i < 5; i++) {
    delay(0);
    std::string message = name + ": value is " + std::to_string(++value) + "\n";
    write(STDOUT_FILENO, message.c_str(), message.size());
    // write above is atomic because message is small
  }
}

int main(int argc, char *argv[]) {
  std::atomic<int> value = 0;
  std::thread t1([&value]() { count("thread t1", value); });
  std::thread t2([&value]() { count("thread t2", value); });
  t1.join(); // thread equivalent of waitpid
  t2.join();
  return 0;
}
