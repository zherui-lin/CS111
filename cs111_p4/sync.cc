
#include "thread.hh"
#include "timer.hh"

void
Mutex::lock()
{
    if (mine())
	throw SyncError("acquiring mutex already locked by this thread");

    // You need to implement the rest of this function
    IntrGuard guard;
    if (Mutex::owning == nullptr) {
        Mutex::owning = Thread::current();
    } else {
        blocked.push(Thread::current());
        Thread::swtch();
    }
}

void
Mutex::unlock()
{
    if (!mine())
	throw SyncError("unlocking mutex not locked by this thread");

    // You need to implement the rest of this function
    IntrGuard guard;
    if (blocked.empty()) {
        Mutex::owning = nullptr;
    } else {
        Thread *unblocked = blocked.front();
        blocked.pop();
        Mutex::owning = unblocked;
        Thread::ready.push(unblocked);
    }
}

bool
Mutex::mine()
{
    // You need to implement this function
    return Thread::current() == Mutex::owning;
}

void
Condition::wait()
{
    if (!m_.mine())
	throw SyncError("Condition::wait must be called with mutex locked");

    // You need to implement the rest of this function
    IntrGuard guard;
    m_.unlock();
    waiting.push(Thread::current());
    Thread::swtch();
    m_.lock();
}

void
Condition::signal()
{
    if (!m_.mine())
	throw SyncError("Condition::signal must be called with mutex locked");

    // You need to implement the rest of this function
    IntrGuard guard;
    if (!waiting.empty()) {
        Thread *wakedUp = waiting.front();
        waiting.pop();
        Thread::ready.push(wakedUp);
    }
}

void
Condition::broadcast()
{
    if (!m_.mine())
	throw SyncError("Condition::broadcast must be called "
                        "with mutex locked");

    // You need to implement the rest of this function
    IntrGuard guard;
    while (!waiting.empty()) {
        Thread *wakedUp = waiting.front();
        waiting.pop();
        Thread::ready.push(wakedUp);
    }
}
