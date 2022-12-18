#include <condition_variable>
#include <mutex>
#include <queue>

// The total number of Zodiac signs
static const int NUM_SIGNS = 12;

// Represents one party, which is capable of matching guests according
// to their zodiac signs.
class Party {
public:
    Party();
    std::string meet(std::string &my_name, int my_sign, int other_sign);

    class Guest {
        public:
            Guest(std::string name): name(name) {}
            std::string name;
            std::condition_variable cv;
            std::string targetName;
    };

private:
    // Synchronizes access to this structure.
    std::mutex mutex;  
    std::queue<Guest *> guests[NUM_SIGNS][NUM_SIGNS];
};

Party::Party()
    : mutex()
{
}

std::string Party::meet(std::string &my_name, int my_sign, int other_sign)
{
    // You need to implement this
    std::unique_lock lock(mutex); 
    if (guests[other_sign][my_sign].empty()) {
        Party::Guest g(my_name);
        guests[my_sign][other_sign].push(&g);
        g.cv.wait(lock);
        return g.targetName;
    }
    Party::Guest *target = guests[other_sign][my_sign].front();
    guests[other_sign][my_sign].pop();
    target->targetName = my_name;
    target->cv.notify_one();
    return target->name;
}
