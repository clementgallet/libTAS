/*
    Copyright 2015-2020 Clément Gallet <clement.gallet@ens-lyon.org>

    This file is part of libTAS.

    libTAS is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    libTAS is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with libTAS.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef LIBTAS_CONCURRENTQUEUE_H_INCLUDED
#define LIBTAS_CONCURRENTQUEUE_H_INCLUDED

#include <queue>
#include <mutex>

/* Thread-safe queue, will be used for a single producer/single consumer model
 * taken from https://juanchopanzacpp.wordpress.com/2013/02/26/concurrent-queue-c11/
 */
template <typename T>
class ConcurrentQueue {
public:

    bool empty() const
    {
        /* We should not need to protect this function with a mutex */
        return queue_.empty();
    }

    typename std::list<T>::const_iterator begin()
    {
        return queue_.cbegin();
    }

    typename std::list<T>::const_iterator end()
    {
        return queue_.cend();
    }
    
    void lock()
    {
        mutex_.lock();
    }

    void unlock()
    {
        mutex_.unlock();
    }

    void pop(T& item)
    {
        std::unique_lock<std::mutex> mlock(mutex_);
        item = queue_.front();
        queue_.pop_front();
    }

    void push(const T& item)
    {
        std::unique_lock<std::mutex> mlock(mutex_);
        queue_.push_back(item);
    }

    ConcurrentQueue()=default;
    ConcurrentQueue(const ConcurrentQueue&) = delete;            // disable copying
    ConcurrentQueue& operator=(const ConcurrentQueue&) = delete; // disable assignment

private:
    std::list<T> queue_;
    std::mutex mutex_;
};

#endif
