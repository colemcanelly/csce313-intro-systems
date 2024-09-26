#include "BoundedBuffer.h"

using namespace std;


BoundedBuffer::BoundedBuffer (int _cap) : cap(_cap) {
    // modify as needed
}

BoundedBuffer::~BoundedBuffer () {
    // modify as needed
}

/**
 * 1. Convert the incoming byte sequence given by msg and size into a vector<char>
 * 2. Wait until there is room in the queue (i.e., queue lengh is less than cap)
 *  -   Using cv for not full, acquire the lock
 * 3. Then push the vector at the end of the queue
 *  -   Unlock the unique_lock
 * 4. Wake up threads that were waiting for push
 */
void BoundedBuffer::push (char* msg, int size)
{
    vector<char> message_vec{msg, msg + size};

    std::unique_lock<std::mutex> push_lock{buffer_mutex};
    not_full.wait(push_lock, [this]{ return q.size() < (long unsigned int) cap; });

    q.push(message_vec);
    push_lock.unlock();

    not_empty.notify_one();
}

/**
 * 1. Wait until the queue has at least 1 item
 *  -   Using the cv for not empty
 * 2. Pop the front item of the queue. The popped item is a vector<char>
 * 3. Convert the popped vector<char> into a char*, copy that into msg; assert that the vector<char>'s length is <= size
 *  -   Unlock()
 * 4. Wake up threads that were waiting for pop
 * 5. Return the vector's length to the caller so that they know how many bytes were popped
 */
int BoundedBuffer::pop (char* msg, int size)
{
    std::unique_lock<std::mutex> pop_lock{buffer_mutex};
    not_empty.wait(pop_lock, [this]{ return q.size() > 0; });

    std::vector<char> message_vec = q.front();
    q.pop();
    pop_lock.unlock();

    assert(message_vec.size() <= (long unsigned int) size);
    memcpy(msg, message_vec.data(), message_vec.size());

    not_full.notify_one();

    return message_vec.size();
}

size_t BoundedBuffer::size () {
    return q.size();
}