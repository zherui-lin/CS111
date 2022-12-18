#include <condition_variable>
#include <mutex>

class Station {
public:
    Station();
    void load_train(int count);
    void wait_for_train();
    void seated();
    
private:
    // Synchronizes access to all information in this object.
    std::mutex mutex;
    int empty;
    int boarding;
    int waiting;
    bool train;
    std::condition_variable cvTrain;
    std::condition_variable cvBoard;
    std::condition_variable cvSeated;
};

Station::Station()
    : mutex(), empty(0), boarding(0), waiting(0), train(false)
{
}

void Station::load_train(int available)
{
    // You need to implement this
    std::unique_lock lock(mutex);
    train = true;
    empty = available;
    boarding = 0;
    cvTrain.notify_all();
    while (waiting != 0 && empty != 0) {
        cvBoard.wait(lock);
    }
    while (boarding != 0) {
        cvSeated.wait(lock);
    }
    train = false;
}

void Station::wait_for_train()
{
    // You need to implement this
    std::unique_lock lock(mutex);
    waiting++;
    while (!train || empty == 0) {
        cvTrain.wait(lock);
    }
    waiting--;
    empty--;
    boarding++;
    cvBoard.notify_one();
}

void Station::seated()
{
    // You need to implement this
    std::unique_lock lock(mutex);
    boarding--;
    cvSeated.notify_one();
}