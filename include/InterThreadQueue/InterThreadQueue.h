#ifndef INTER_THREAD_QUEUE_H
#define INTER_THREAD_QUEUE_H

#include <vector>
#include <mutex>

template <typename Class>
class InterThreadQueue{
public:
    void start_data_access(){
        queue_mutex.lock();
    }

    void end_data_access(){
        queue_mutex.unlock();
    }

    auto begin() -> decltype(std::vector<Class>().begin()){
        return queue.begin();
    };

    auto end() -> decltype(std::vector<Class>().end()){
        return queue.end();
    };

    void push(const Class& el){
        queue_mutex.lock();
        queue.push_back(el);
        queue_mutex.unlock();
    };

    void push(const std::vector<Class>& el_vec){
        queue_mutex.lock();
        queue.insert(queue.end(), el_vec.begin(), el_vec.end());
        queue_mutex.unlock();
    };

    void clear(){
        queue_mutex.lock();
        queue.clear();
        queue_mutex.unlock();
    }

    bool empty(){
        return queue.empty();
    }

private:
    std::mutex queue_mutex;
    std::vector<Class> queue;
};

#endif