/*
 * LibreTuner
 * Copyright (C) 2018  Altenius
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "timerrunloop.h"

#include <algorithm>

TimerRunLoop::TimerRunLoop() : running_(false) {}

TimerRunLoop & TimerRunLoop::get()
{
    static TimerRunLoop trl;
    return trl;
}

void TimerRunLoop::addTimer(const std::shared_ptr<Timer> & timer)
{
    std::lock_guard<std::mutex> lk(mutex_);
    queue_.insert(timer);
    wake_.notify_all();
}

void TimerRunLoop::removeTimer(const std::shared_ptr<Timer> & timer)
{
    std::lock_guard<std::mutex> lk(mutex_);
    queue_.erase(std::weak_ptr<Timer>(timer));
    wake_.notify_all();
}

void TimerRunLoop::runLoop()
{
    std::unique_lock<std::mutex> lk(mutex_);
    while (running_)
    {
        if (queue_.empty())
        {
            wake_.wait(lk);
            continue;
        }

        auto begin = queue_.begin();
        std::chrono::steady_clock::time_point nextTrigger;
        if (auto ptr = begin->lock())
        {
            lk.unlock();
            if (ptr->tryTrigger())
            {
                lk.lock();
                continue;
            }
            nextTrigger = ptr->nextTrigger();
            /* We don't want to keep the shared_ptr around while waiting, so
             * wait and then lock the weak pointer again */
            lk.lock();
        }
        else
        {
            // Dead
            queue_.erase(begin);
            continue;
        }
        std::weak_ptr<Timer> weak = *begin;
        wake_.wait_until(lk, nextTrigger);
        lk.unlock();
        if (auto ptr = weak.lock())
        {
            ptr->tryTrigger();
        }
        lk.lock();
    }
}

void TimerRunLoop::startWorker()
{
    if (worker_.joinable())
    {
        // Already working; abort
        return;
    }
    running_ = true;
    worker_ = std::thread(std::bind(&TimerRunLoop::runLoop, this));
}

void TimerRunLoop::stopWorker()
{
    running_ = false;
    wake_.notify_all();
    worker_.join();
}

TimerRunLoop::~TimerRunLoop()
{
    if (worker_.joinable())
    {
        stopWorker();
    }
}
