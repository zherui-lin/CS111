
#include <unistd.h>

#include "stack.hh"
#include "thread.hh"
#include "timer.hh"

Thread *Thread::initial_thread = new Thread(nullptr);
Thread *Thread::curr = initial_thread;
std::queue<Thread *> Thread::ready;

// Create a placeholder Thread for the program's initial thread (which
// already has a stack, so doesn't need one allocated).
Thread::Thread(std::nullptr_t)
{
    // You have to implement this; depending on the contents of your
    // Thread structure, an empty function body may be sufficient.
}

Thread::~Thread()
{
    // You have to implement this
}

void
Thread::start() {
    intr_enable(true);
    Thread::current()->cur_main();
    Thread::exit();
}

void
Thread::create(std::function<void()> main, size_t stack_size)
{
    // You have to implement this
    IntrGuard guard;
    Thread *t = new Thread(nullptr);
    t->cur_main = main;
    t->stack = (Bytes) new char[stack_size];
    t->sp = stack_init(t->stack.get(), stack_size, start);
    t->schedule();
}

Thread *
Thread::current()
{
    // Replace the code below with your implementation.
    return curr;
}

void
Thread::schedule()
{
    // You have to implement this
    IntrGuard guard;
    std::queue<Thread *> copy(ready);
    while (!copy.empty()) {
        if (copy.front() == this) {
            return;
        }
        copy.pop();
    }
    ready.push(this); 
}

void
Thread::swtch()
{
    // You have to implement this
    IntrGuard guard;
    if (ready.empty()) {
        std::exit(0);
    }
    Thread *prev = curr;
    curr = ready.front();
    ready.pop();
    stack_switch(&prev->sp, &curr->sp);
}

void
Thread::yield()
{
    // You have to implement this
    IntrGuard guard;
    curr->schedule();
    swtch();
}

void
Thread::exit()
{
    // You have to implement this
    IntrGuard guard;
    if (curr != initial_thread) {
        delete curr;
    }
    swtch();
    std::abort();  // Leave this line--control should never reach here
}

void
Thread::preempt_init(std::uint64_t usec)
{
    // You have to implement this
    timer_init(usec, yield);
}
